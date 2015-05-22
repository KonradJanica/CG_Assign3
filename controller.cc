#include "controller.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
Controller::Controller(const Renderer * r, const bool &debug_flag) 
  : renderer_(r), light_pos_(glm::vec4(0,0,0,0)), delta_time_(0), last_frame_(0), is_debugging_(debug_flag) {
  camera_ = new Camera;
  is_key_pressed_hash_.reserve(256);
  is_key_pressed_hash_.resize(256);
}

// Adds a model to the member vector
//   @param program_id, a shader program
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
//   TODO split Car into it's own
void Controller::AddModel(const GLuint &program_id, const std::string &model_filename, const bool &is_car) {
  if (is_car) {
    car_ = new Model(program_id, model_filename, 
                     glm::vec3(0.0f, 0.3f, 15.0f), // Translation  move behind first tile (i.e. start on 2nd tile)
                     glm::vec3(0.0f, 0.0f, 0.0f),  // Rotation
                     glm::vec3(0.3f, 0.3f, 0.3f),  // Scale
                     0, false); // starting speed and debugging mode
  } else {
    Object * object = new Model(program_id, model_filename, glm::vec3(0,0,0));
    objects_.push_back(object);
  }
}

// Renders all models in the vector member
//   Should be called in the render loop
void Controller::Draw() {
  // Lights need to be transformed with view/normal matrix
  PositionLights();

  // Spider-man
  renderer_->Render(objects_.at(0), camera_);
  // Car with physics
  renderer_->Render(car_, camera_);
  // Terrain
  renderer_->Render(terrain_, camera_);

  renderer_->RenderWater(terrain_, camera_, light_pos_);
  // Axis
  // TODO Toggle
  renderer_->RenderAxis(camera_);
}

// Assumes SetupLighting() has been called, only updates essential light properties
void Controller::PositionLights() {
  glm::mat4 view_matrix = camera_->view_matrix();
  glm::mat4 car_mv_matrix = view_matrix * car_->model_matrix();
  glm::mat3 norm_matrix = glm::mat3(view_matrix);

  DirectionalLight dirLight = directional_light_;
  dirLight.Direction = norm_matrix * dirLight.Direction;

  // TODO: Refactor creation of light. Split creation from re-positioning
  SpotLight spotLight[1];
  spotLight[0].AmbientIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
  spotLight[0].DiffuseIntensity = glm::vec3(1.0f, 1.0f, 1.0f);
  spotLight[0].SpecularIntensity = glm::vec3(1.0f, 1.0f, 1.0f);

  glm::mat4 spotLightTranslation0 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.45f));
  glm::mat3 spotLightNormMatx0 = glm::mat3(car_mv_matrix);

  spotLight[0].Position = glm::vec3(car_mv_matrix * spotLightTranslation0 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
  spotLight[0].Direction = spotLightNormMatx0 * glm::vec3(0.0f, -0.3f, 1.0f);
  spotLight[0].CosineCutoff = cos(DEG2RAD(20.0f));
  spotLight[0].Attenuation.Constant = 0.5f;
  spotLight[0].Attenuation.Linear = 0.1f;
  spotLight[0].Attenuation.Exp = 0.01f;

  PointLight pointLights[2];
  for (unsigned int i = 0; i < 2; i++) {
    pointLights[i].DiffuseIntensity = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::mat4 pointLightTranslation0 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f + i * 1.0f, 0.5f, -3.0f));
    pointLights[i].Position = glm::vec3(car_mv_matrix * pointLightTranslation0 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    pointLights[i].Attenuation.Constant = 0.01f;
    pointLights[i].Attenuation.Linear = 3.0f;
    pointLights[i].Attenuation.Exp = 5.0f;
  }

  light_controller_->SetDirectionalLight(car_->program_id(), dirLight);
  light_controller_->SetSpotLights(car_->program_id(), 1, spotLight);
  light_controller_->SetPointLights(car_->program_id(), 2, pointLights);
}

// Setup Light Components into Uniform Variables for Shader
void Controller::SetupLighting(const GLuint &program_id) {
  light_controller_ = new LightController();

  // Send directional light
  directional_light_.AmbientIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
  directional_light_.DiffuseIntensity = glm::vec3(0.3f, 0.3f, 0.3f);
  // directional_light_.DiffuseIntensity = glm::vec3(0.4f, 0.4f, 0.7f);
  directional_light_.SpecularIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
  // directional_light_.SpecularIntensity = glm::vec3(0.5f, 0.5f, 0.5f);
  directional_light_.Direction = glm::vec3(0.3f, -1.0f, -0.3f);


  

  // light_controller_->SetDirectionalLight(dirLight);
}

// Creates the Terrain object for RenderTerrain()
//   Creates Terrain VAOs
//   @warn terrain_ on heap, must be deleted after
void Controller::EnableTerrain(const GLuint &program_id, const GLuint &water_id) {
  terrain_ = new Terrain(program_id, water_id);
}

// The main control tick
//   Controls everything: camera, inputs, physics, collisions
void Controller::UpdateGame() {
  // calculate delta time
  GLfloat current_frame = glutGet(GLUT_ELAPSED_TIME);
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  UpdateCamera();
  UpdatePhysics();
}

// The controllers camera update tick
//   Uses car position (for chase and 1st person view)
//   and checks keypresses for freeview
//   @warn should be called before car movement
void Controller::UpdateCamera() {
  // CAMERA CONTROLS
  // Freeview movement
  camera_->Movement(delta_time_, is_key_pressed_hash_); 
  // Point at car
  camera_->UpdateCarTick(car_);
  // Update camera lookAt
  camera_->UpdateCamera();
}

// The controllers physics update tick
//   Checks keypresses and calculates acceleration
void Controller::UpdatePhysics() {

  // Sets variables required for camera
  //  i.e. camera has access to car in UpdateCarTick(car_)
  car_->ControllerMovementTick(delta_time_, is_key_pressed_hash_);

  ///////////////// COLLISION TESTING TODO TODO
  typedef std::unordered_map<float,std::pair<float,float>>  col_map;
  const std::list<col_map> &col = terrain_->collision_queue_hash();
  const glm::vec3 &new_translation = car_->translation();
  float car_z = new_translation.z;
  car_z = round(car_z);
  // printf("car_z = %f\n", car_z);
  const col_map &road_tile1  = col.front();
  col_map::const_iterator got = road_tile1.find(car_z);
  if (got == road_tile1.end()) {
    printf("possible collision on Z, check next tile\n");
    std::list<col_map>::const_iterator it = col.begin();
    it++;
    const col_map &road_tile2 = *(it);
    got = road_tile2.find(car_z);
    if (got == road_tile2.end()) {
      printf("collision on x!\n");
    } else {
      printf("assuming end of tile reached, pop!\n");
      // TODO fix check end of tile...
      terrain_->col_pop();
      // TODO obviously this needs to be done further back from view space
      terrain_->ProceedTiles();
    }
  } else {
    // Check if x is in range
    float min_x = got->second.first;
    float max_x = got->second.second;
    float car_x = new_translation.x;
    if (car_x >= min_x && car_x <= max_x) {
      //inside bounds
    } else {
      printf("collision on x!\n");
    }
  }

}

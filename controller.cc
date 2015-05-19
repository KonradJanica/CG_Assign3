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
  
  if(model_filename == "SKYBOX")
  {
    printf("WE ARE ADDING A SKYBOX \n");
    skybox_ = new Model(program_id, model_filename, 1,
                     glm::vec3(0.0f, 5.0f, 10.0f), // Translation
                     glm::vec3(0.0f, 0.0f, 0.0f),  // Rotation
                     glm::vec3(1.0f, 1.0f, 1.0f)); // Scale
    return;
  }

  if (is_car) {
    car_ = new Model(program_id, model_filename, 
                     glm::vec3(0.0f, 0.3f, 10.0f), // Translation
                     glm::vec3(0.0f, 0.0f, 0.0f),  // Rotation
                     glm::vec3(0.3f, 0.3f, 0.3f)); // Scale
    car_->EnablePhysics(0.0001,0.00001,0);
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

  renderer_->RenderSky(skybox_, camera_);
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

  light_controller_->SetDirectionalLight(dirLight);
  light_controller_->SetSpotLights(1, spotLight);
  light_controller_->SetPointLights(2, pointLights);
}

// Setup Light Components into Uniform Variables for Shader
void Controller::SetupLighting(const GLuint &program_id) {
  light_controller_ = new LightController();
  if (!light_controller_->Init(program_id)) {
    fprintf(stderr, "Error initialising light controller\n");
    exit(0);
  }

  // TODO @MITCH FOR CTRL F
  // Send directional light
  directional_light_.AmbientIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
  directional_light_.DiffuseIntensity = glm::vec3(1.0f, 1.0f, 1.0f);
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

// The controllers physics update tick
//   Checks keypresses and calculates acceleration
void Controller::UpdatePhysics() {

  // TODO remove this and make only 1 delta time calculation
  GLfloat current_frame = glutGet(GLUT_ELAPSED_TIME);
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  float MOVESPEED = 0.004f * delta_time_;
  float TURNRATE = 0.2f * delta_time_;

  // Used to calculate car movement
  glm::vec3 new_translation = car_->translation();
  glm::vec3 rotation = car_->rotation();
  // Used to update camera
  glm::vec3 displacement = glm::vec3(0,0,0);

  if (is_key_pressed_hash_.at('w')) {
    float x_movement = MOVESPEED * sin(DEG2RAD(rotation.y));
    float z_movement = MOVESPEED * cos(DEG2RAD(rotation.y));
    new_translation.x += x_movement;
    new_translation.z += z_movement;
    car_->set_translation(new_translation);
    displacement.x += x_movement;
    displacement.z += z_movement;
    //TODO @ MICTH FLAGS FOR CNTRL F
    glm::vec3 sky_translation = skybox_->translation();
    sky_translation.z += MOVESPEED;
    skybox_->set_translation(sky_translation);
    //skybox_->UpdateModelMatrix();
  }
  if (is_key_pressed_hash_.at('s')) {
    float x_movement = MOVESPEED * sin(DEG2RAD(rotation.y));
    float z_movement = MOVESPEED * cos(DEG2RAD(rotation.y));
    new_translation.x -= x_movement;
    new_translation.z -= z_movement;
    car_->set_translation(new_translation);
    displacement.x -= x_movement;
    displacement.z -= z_movement;

    glm::vec3 sky_translation = skybox_->translation();
    sky_translation.z -= MOVESPEED;
    skybox_->set_translation(sky_translation);
    skybox_->UpdateModelMatrix();
  }
  car_->set_displacement(displacement);

  // No Acceleration
  if (!is_key_pressed_hash_.at('w') && !is_key_pressed_hash_.at('s')) {
    // car_->Accelerate(0);
  }
  if (is_key_pressed_hash_.at('a')) {
    car_->set_rotation(glm::vec3(car_->rotation().x, car_->rotation().y + TURNRATE, car_->rotation().z));
  }
  if (is_key_pressed_hash_.at('d')) {
    car_->set_rotation(glm::vec3(car_->rotation().x, car_->rotation().y - TURNRATE, car_->rotation().z));
  }

  ///////////////// COLLISION TESTING TODO TODO
  const std::queue<std::unordered_map<float,std::pair<float,float>>> &col = terrain_->collision_queue_hash();
  float car_z = new_translation.z;
  car_z = round(car_z);
  // printf("car_z = %f\n", car_z);
  std::unordered_map<float,std::pair<float,float>> road_tile1;
  road_tile1 = col.front();
  std::unordered_map<float,std::pair<float,float>>::const_iterator got = road_tile1.find(car_z);
  if (got == road_tile1.end()) {
    // printf("possible collision on Z, check next tile");
    printf("assuming end of tile reached, pop!\n");
    // TODO fix check end of tile...
    terrain_->col_pop();
    // TODO obviously this needs to be done further back from view space
    terrain_->ProceedTiles();
  } else {
    // Check if x is in range
    float min_x = got->second.first;
    float max_x = got->second.second;
    float car_x = new_translation.x;
    if (car_x >= min_x && car_x <= max_x) {
      //inside bounds
    } else {
      printf("collision on x! %f\n", current_frame);
    }
  }
  // printf("%f\n", current_frame);

  ///////////////// CAMERA CONTROLS
  // point at car
  camera_->UpdateCarTick(car_);
}

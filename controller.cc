#include "controller.h"
#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

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
                     glm::vec3(0.0f, 0.3f, 10.0f), // Translation
                     glm::vec3(0.0f, 0.0f, 0.0f),  // Rotation
                     glm::vec3(0.3f, 0.3f, 0.3f)); // Scale
    car_->EnablePhysics(0.0001,0.00001,0);
  } else {
    Object * object = new Model(program_id, model_filename, glm::vec3(0,0,0));
    objects_.push_back(object);
  }

  SetupLighting(program_id, glm::vec3(0,0,0), glm::vec3(0.7,0.7,1), glm::vec3(1,1,1));
}

// Renders all models in the vector member
//   Should be called in the render loop
void Controller::Draw() {
  // Spider-man
  renderer_->Render(objects_.at(0), camera_, light_pos_);
  // Car with physics
  renderer_->Render(car_, camera_, light_pos_);
  // Terrain
  renderer_->Render(terrain_, camera_, light_pos_);

  renderer_->RenderWater(terrain_, camera_, light_pos_);
  // Axis
  // TODO Toggle
  renderer_->RenderAxis(camera_);
}

// Setup Light Components into Uniform Variables for Shader
void Controller::SetupLighting(const GLuint &program_id, const glm::vec3 &light_ambient, const glm::vec3 &light_diffuse, const glm::vec3 &light_specular, const GLint &light_toggle_in) {
  glUseProgram(program_id);
  // Uniform lighting variables
  int lightambientHandle = glGetUniformLocation(program_id, "light_ambient");
  int lightdiffuseHandle = glGetUniformLocation(program_id, "light_diffuse");
  int lightspecularHandle = glGetUniformLocation(program_id, "light_specular");
  int lightToggleHandle = glGetUniformLocation(program_id, "light_toggle");
  if ( lightambientHandle == -1 ||
      lightdiffuseHandle == -1 ||
      lightspecularHandle == -1 ||
      lightToggleHandle == -1) {
    printf("Error: can't find light uniform variables, Model.cc approx line 90\n");
  }

  float lightambient[3] = { light_ambient.x, light_ambient.y, light_ambient.z };	// ambient light components
  float lightdiffuse[3] = { light_diffuse.x, light_diffuse.y, light_diffuse.z };	// diffuse light components
  float lightspecular[3] = { light_specular.x, light_specular.y, light_specular.z };	// specular light components
  GLint light_toggle = light_toggle_in;

  glUniform3fv(lightambientHandle, 1, lightambient);
  glUniform3fv(lightdiffuseHandle, 1, lightdiffuse);
  glUniform3fv(lightspecularHandle, 1, lightspecular);    
  glUniform1i(lightToggleHandle, light_toggle);    
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

  if (is_key_pressed_hash_.at('w')) {
    glm::vec3 new_translation = car_->translation();
    glm::vec3 rotation = car_->rotation();
    new_translation.x += MOVESPEED * sin(DEG2RAD(rotation.y));
    new_translation.z += MOVESPEED * cos(DEG2RAD(rotation.y));
    car_->set_translation(new_translation);
  }
  if (is_key_pressed_hash_.at('s')) {
    glm::vec3 new_translation = car_->translation();
    glm::vec3 rotation = car_->rotation();
    new_translation.x -= MOVESPEED * sin(DEG2RAD(rotation.y));
    new_translation.z -= MOVESPEED * cos(DEG2RAD(rotation.y));
    car_->set_translation(new_translation);
  }
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
  glm::vec3 new_translation = car_->translation();
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
}

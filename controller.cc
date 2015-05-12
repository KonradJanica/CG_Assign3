#include "controller.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
Controller::Controller(const Renderer * r, const bool &debug_flag) 
  : renderer_(r), light_pos_(glm::vec4(0,0,0,0)), is_debugging_(debug_flag) {
  camera_ = new Camera;
}

// Adds a model to the member vector
//   @param program_id, a shader program
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
//   TODO split Car into it's own
void Controller::AddModel(const GLuint &program_id, const std::string &model_filename, const bool &is_car) {
  if (is_car) {
    car_ = new Model(program_id, model_filename, glm::vec3(0.0,0.0,0.0));
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
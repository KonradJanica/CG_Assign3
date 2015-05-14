#ifndef ASSIGN3_CONTROLLER_H_
#define ASSIGN3_CONTROLLER_H_

#include <string>
#include <vector>
#include <cassert>
#include "model_data.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"
#include "object.h"
#include "renderer.h"
#include "light_controller.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "lib/shader/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Controller {
  public:
    // Enum for vertex coordinates
    //   used in GetMax
    enum CoordEnum {
      kX = 0,
      kY = 1, 
      kZ = 2,
      kx = 0, 
      ky = 1, 
      kz = 2,
      kMin = 3,
      kMax = 3,
    };  

    // Construct with verbose debugging mode
    Controller(const Renderer * r, const bool &debug_flag = false);

    // Add a wireframe model from .obj file to the scene
    void AddModel(const GLuint &program_id, const std::string &model_filename, const bool &is_car = false);
    // Render the scene (all member models)
    //   @warn uses the renderer object
    void Draw();
    // Creates the Terrain object for RenderTerrain()
    //   Creates Terrain VAOs
    //   @warn terrain_ on heap, must be deleted after
    void EnableTerrain(const GLuint &program_id, const GLuint &water_id);
    // Setup Light Components into Uniform Variables for Shader
    void SetupLighting(const GLuint &program_id);
    // Set the position of the Light
    inline void SetLightPosition(const float &x, const float &y, const float &z, const float &w);
    // Accessor for largest vertex in indexed model
    //   @return max_$_, the maximum cartesian coordinate of given input
    inline float GetMax(unsigned int index, int e_numb) const;
    // Accessor for smallest vertex in indexed model
    //   @return min_$_, the minimum cartesian coordinate of given input
    inline float GetMin(unsigned int index, int e_numb) const;
    // Accessor for Camera Object
    inline Camera * camera();

    // CONTROLS
    // The controllers physics update tick
    //   Checks keypresses and calculates acceleration
    //   TODO not sure if this belongs in public, can be used for lighting too etc.
    void UpdatePhysics();
    // Trues the key hash on key down event
    inline void KeyPressed(const int &key);
    // Falses the key hash on key down event
    inline void KeyReleased(const int &key);

  private:
    // The renderer reference
    const Renderer * renderer_;

    // All the static models and their transforms in the scene
    std::vector<Object *> objects_;
    // The moving car
    //   An object with the physics extension enabled
    Object * car_;
    // The camera object
    Camera * camera_;
    // The terrain object
    Terrain * terrain_;

    // The light controller
    LightController * light_controller_;

    // Lights
    DirectionalLight directional_light_;
    std::vector<PointLight> point_lights_; 
    std::vector<SpotLight> spot_lights_;

    // Updates light properties with view matrix from camera
    void PositionLights();

    // The shader to use to render Axis Coordinates
    GLuint axis_program_id;

    // The position of the Light for lighting
    glm::vec4 light_pos_;

    float delta_time_;
    float last_frame_;

    // Hash representing keys pressed
    std::vector<bool> is_key_pressed_hash_;

    // Verbose Debugging mode
    bool is_debugging_;
};

// Set the position of the Light
inline void Controller::SetLightPosition(const float &x, const float &y, const float &z, const float &w) {
  light_pos_.x = x;
  light_pos_.y = y;
  light_pos_.z = z;
  light_pos_.w = w;
}
// Accessor for largest vertex in indexed model
//   @param index of member model
//   @param enum value
//   @return max_$_, the maximum cartesian coordinate of given input
inline float Controller::GetMax(unsigned int index, int e_numb) const {
  assert(index < objects_.size() && "Trying to access Models_ out of range");
  switch(e_numb) {
    case 0: 
      return objects_.at(index)->GetMax(0);
    case 1: 
      return objects_.at(index)->GetMax(1);
    case 2:
      return objects_.at(index)->GetMax(2);
    case 3:
      return objects_.at(index)->GetMax(3);
    default:
      assert(false);
  }
}
// Accessor for smallest vertex in indexed model
//   @param index of member model
//   @param enum value
//   @return min_$_, the minimum cartesian coordinate of given input
inline float Controller::GetMin(unsigned int index, int e_numb) const {
  assert(index < objects_.size() && "Trying to access Models_ out of range");
  switch(e_numb) {
    case 0: 
      return objects_.at(index)->GetMin(0);
    case 1: 
      return objects_.at(index)->GetMin(1);
    case 2:
      return objects_.at(index)->GetMin(2);
    case 3:
      return objects_.at(index)->GetMin(3);
    default:
      assert(false);
  }
}
// Accessor for Camera Object
inline Camera * Controller::camera() {
  return camera_;
}
// Trues the key hash on key down event
//   @param a key corresponding to is_key_pressed_hash_
inline void Controller::KeyPressed(const int &key) {
  is_key_pressed_hash_.at(key) = true;
}
// Falses the key hash on key down event
//   @param a key corresponding to is_key_pressed_hash_
inline void Controller::KeyReleased(const int &key) {
  is_key_pressed_hash_.at(key) = false;
}

#endif

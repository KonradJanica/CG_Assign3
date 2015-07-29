#ifndef ASSIGN3_CONTROLLER_H_
#define ASSIGN3_CONTROLLER_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "model_data.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"
#include "object.h"
#include "car.h"
#include "renderer.h"
#include "sun.h"
#include "light_controller.h"
#include "roadsign.h"
#include "collision_controller.h"
#include "npc_car_controller.h"
#include "Skybox.h"
#include "Water.h"
#include "rain.h"

#include "constants.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "shaders/shader_compiler/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h>
#endif

class Controller {
  public:

    // Construct with window dimensions & verbose debugging mode
    Controller(const int window_width, const int window_height, const bool debug_flag = false);

    // Creates an object for the member vector
    //   @param shader, a shader class holding shader to use and uniforms
    //   @param model_filename, a string containing the path of the .obj file
    //   @warn the model is created on the heap and memory must be freed afterwards
    Object * AddObject(const Shader & shader, const std::string &model_filename);
    // Creates a car for the member vector
    //   @param shader, a shader class holding shader to use and uniforms
    //   @param model_filename, a string containing the path of the .obj file
    //   @warn the model is created on the heap and memory must be freed afterwards
    Car * AddCar(const Shader & shader, const std::string &model_filename);

    // Render the scene (all member models)
    //   @warn uses the renderer object
    //   // TODO const?
    void Draw();
    // Set the position of the Light
    inline void SetLightPosition(const float &x, const float &y, const float &z, const float &w);
    // Accessor for Camera Object
    inline Camera * camera();

    // TICKS
    // The main control tick
    //   Controls everything: camera, inputs, physics, collisions
    void UpdateGame();

    // CONTROLS
    // Trues the key hash on key down event
    inline void KeyPressed(const int &key);
    // Falses the key hash on key down event
    inline void KeyReleased(const int &key);

  private:
    // OBJECTS
    // The renderer reference
    const Renderer renderer_;
    // The shaders object (holds and compiles all shaders)
    const Shaders * shaders_;
    // The camera object
    Camera camera_;
    // The Sun object to control environment lighting
    Sun sun_;
    // The light controller
    LightController * light_controller_;
    // The Player Car Collision Controller
    //   Detects crashes, proceeds circular tile vector
    //   Plays crash animation
    //   Finds and holds middle of road
    //   Holds car direction near car
    CollisionController collision_controller_;
    // The NPC Car Collision Controller
    //   Detects crashes on player vs npc cars
    //   Plays crash animation
    NpcCarController npc_car_controller_;

    // Updates light properties with view matrix from camera

    // The terrain object
    Terrain * terrain_;
    // The road sign factory
    //   Holds all road signs and has members to position them
    RoadSign road_sign_;
    // The player moving car
    //   An object with physics
    Car * const car_;
    // The skybox object
    Skybox * skybox_;
    // The water object
    Water * water_;
    // The rain object
    Rain * rain_;

    bool playSound;

    // The current game state
    kGameState game_state_;

    // INTERNAL TICKS
    // The controllers camera update tick
    //   Uses car position (for chase and 1st person view)
    //   and checks keypresses for freeview
    //   @warn should be called before car movement
    void UpdateCamera();
    // The controllers physics update tick
    //   Checks keypresses and calculates acceleration
    void UpdatePhysics();

    void PositionLights();

    // The shader to use to render Axis Coordinates
    GLuint axis_program_id;

    // The position of the Light for lighting
    glm::vec4 light_pos_;

    // FPS and Smoothing Vars
    unsigned long long frames_past_;
    int frames_count_;
    GLfloat delta_time_;

    // Hash representing keys pressed
    std::vector<bool> is_key_pressed_hash_;

    // Verbose Debugging mode
    const bool is_debugging_;
};

// Set the position of the Light
inline void Controller::SetLightPosition(const float &x, const float &y, const float &z, const float &w) {
  light_pos_.x = x;
  light_pos_.y = y;
  light_pos_.z = z;
  light_pos_.w = w;
}
// Accessor for Camera Object
inline Camera * Controller::camera() {
  return &camera_;
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

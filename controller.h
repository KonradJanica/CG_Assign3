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
#include "renderer.h"
#include "light_controller.h"
#include "Skybox.h"
#include "Water.h"
#include "rain.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "lib/shader/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h>
#endif

#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

class Controller {
  public:
    // Enum for game states
    enum State {
      kStart = 0,
      kPause = 1,
      kResume = 2,
      kAutoDrive = 3,
      kCrashingFall = 4,
      kCrashingCliff = 5,
      kGameOver = 6,
    };

    // Construct with verbose debugging mode
    Controller(const Renderer * r, const bool &debug_flag = false);

    void AddSkybox(const GLuint program_id);

    void AddWater(const GLuint program_id);

    void AddRain(GLuint program_id);

    // Add a wireframe model from .obj file to the scene
    void AddModel(const GLuint program_id, const std::string &model_filename, const bool &is_car = false);
    // Render the scene (all member models)
    //   @warn uses the renderer object
    void Draw();
    // Creates the Terrain object for RenderTerrain()
    //   Creates Terrain VAOs
    //   @warn terrain_ on heap, must be deleted after
    void EnableTerrain(const GLuint program_id);
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
    // The renderer reference
    const Renderer * renderer_;

    // All the static models and their transforms in the scene
    std::vector<Object *> objects_;
    // The moving car
    //   An object with physics
    Object * car_;
    // The camera object
    Camera * camera_;
    // The terrain object
    Terrain * terrain_;
    // The skybox object
    Skybox * skybox_;
    // The water object
    Water * water_;
    // The rain object
    Rain * rain_;

    bool playSound;

    // The current game state
    State game_state_;
    // The users camera state (for animations)
    Camera::State camera_state_;
    // The state of the previous collision tick
    bool is_collision_;
    // The midpoint of the road where the car is
    glm::vec3 left_lane_midpoint_;
    // The previous midpoint, updated during autodrive
    glm::vec3 prev_left_lane_midpoint_;
    // The direction vector of the road where the car is
    glm::vec3 road_direction_;
    // The rotation of the road where the car is
    float road_y_rotation_;
    // The angle of the car and the direction of road
    //   Angle is clockwise from facing of road
    //   This includes the cars centripetal direction
    float car_angle_;
    // Has the car hit the right side cliff yet?
    //   Used in left (cliff) animation
    bool is_cliff_hit_;
    // Used in left (cliff) animation for collision detection using determinate
    bool is_prev_positive_;
    // The impact speed of the car for the left (cliff) side animation
    float impact_speed_;

    // INTERNAL TICKS
    // The controllers camera update tick
    //   Uses car position (for chase and 1st person view)
    //   and checks keypresses for freeview
    //   @warn should be called before car movement
    void UpdateCamera();
    // The controllers physics update tick
    //   Checks keypresses and calculates acceleration
    void UpdatePhysics();

    // TODO comment
    unsigned char prev_colisn_pair_idx_;
    // TODO comment
    void UpdateCollisions();
    // COLLISION HELPERS
    // Checks whether car is between boundary pair
    //   Creates 4 triangles out of the 4 points of the given square and returns 
    //   @param car, the car object (to find it's position)
    //   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
    //   @return true if car is inside corner of rectangle
    //   @warn input must be square for accurate results
    bool IsInside(const glm::vec3 &car, const std::pair<Terrain::boundary_pair,Terrain::boundary_pair> &bp);
    // Checks whether car is between a pair of points
    //   @param car, the car vec3 (to find it's position)
    //   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
    //   @return true if car is within the points
    bool IsInside(const glm::vec3 &car, const std::pair<glm::vec3,glm::vec3> &bp);

    float colisn_anim_ticks_;
    // The animation played when the car falls off the right (water) side
    //   Is calculated using the vertices stored by terrain
    //   Finds the closest vertice to car and doesn't allow it to go below it
    //   Once complete resets the state to kAutoDrive
    // @warn Pretty inefficent way of checking for collisions but it's only
    //       calculated during this state.
    void CrashAnimationFall();
    // The animation played when the car drives off the road on left (cliff) side
    //   Is calculated using 1 row of vertices stored by terrain
    //   Finds the closest vertice to car and calculates determinate, when determinate
    //     of middle of road and car position are opposite there is a colisn
    //   A recovery occurs when both determinates are opposite but closest dis is too far
    // @warn This collision can fall through if delta time makes the car velocity larger
    //       than the catch dis, may be an issue for systems with poor performance
    void CrashAnimationCliff();

    // The light controller
    LightController * light_controller_;

    // Updates light properties with view matrix from camera
    void PositionLights();

    // The shader to use to render Axis Coordinates
    GLuint axis_program_id;

    // The position of the Light for lighting
    glm::vec4 light_pos_;

    // FPS and Smoothing Vars
    int frames_count_;
    unsigned long long frames_past_;
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

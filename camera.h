/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * camera.h, definition of multiple camera types in openGL
 * 
 * This file is a definition of different camera types in openGL
 * including a chase cam, first person cam and a freeview cam
 *
 * Refer to the comments describing each function for further detail
 *
 * 
 */

#ifndef ASSIGN3_CAMERA_H_
#define ASSIGN3_CAMERA_H_

#include <vector>
#include <string>
#include <cassert>
#include "model_data.h"
#include "model.h"
#include "object.h"

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

#define kPi 3.14159265358979323846
#define k89DegreesToRadians 1.55334303

#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

class Camera {
  public:
    // CONSTANTS:
    enum State {
      kFreeView,
      kChase,
      kFirstPerson,
    };

    // CONSTRUCTORS:
    // Default Constructor sets starting position of Camera at 0,0,3
    Camera();

    // MUTATORS:
    // Changes the current state of the camera, hence changes the viewing
    // mode and orientation
    void CycleState();
    // Changes the current state of the camera to the specified state
    //   @param State, the mode and orientation state of the camera
    inline void ChangeState(State state);
    // Better method using is_key_pressed_hash for multiple inputs
    void Movement(float delta_time, const std::vector<bool> &is_key_pressed_hash);
    // Changes Direction by X and Y mouse inputs
    //   Calculates the difference of previous mouse positions
    //   and current to work out the new direction
    //   @param int x, new mouse x position
    //   @param int y, new mouse y position
    //   @warn relies on UpdatePreviousMouse(x,y) to work
    void ChangeDirection(int x, int y);
    // Changes Aspect ratio to Zoom the camera
    //   Calculates the difference of previous mouse y positions
    //   and current to work out the new aspect
    //   @param int y, new mouse y position
    //   @warn relies on UpdatePreviousMouse(x,y) to work
    void ChangeZoom(const int &y);
    // Update upon Car movement tick
    //   Called in Car's physics tick
    void UpdateCarTick(const Object * car);
    // Update the Camera Matrix
    void UpdateCamera();
    // Mutates the cameras position
    inline void ResetPosition(const glm::vec3 &camera_position);

    // ACCESSORS:
    // Accessor for the camera matrix
    inline glm::mat4 view_matrix() const;
    // Accessor for the Current Zoom Aspect Ratio
    //  To be used in render
    inline float aspect() const;
    // Accessor for the Current Camera Position
    inline glm::vec3 cam_pos() const;
    // Accessor for the Current Camera State
    inline State state() const;

    // MOUSE SETTINGS:
    // Left and Right Button States
    inline void set_is_left_button(const bool &state);
    inline void set_is_right_button(const bool &state);
    inline bool is_left_button() const;
    inline bool is_right_button() const;
    // Set both X and Y previous positions of Mouse
    inline void UpdatePreviousMouse(const int &x, const int &y);
    // Access X and Y previous positions of Mouse
    inline int prev_mouse_x() const;
    inline int prev_mouse_y() const;
    
  private:
    // The viewing state of the camera
    State state_;
    // The modelview matrix for the viewing camera
    glm::mat4 view_matrix_;
    // The Position of the Camera
    glm::vec3 cam_pos_;
    // The Front (or back depends on your visualization) position of the camera
    glm::vec3 cam_front_;
    // The upwards vector of the camera
    glm::vec3 cam_up_;
    // Aspect Ratio used for zooming
    float aspect_;
    // The Yaw (Up/Down) of the Camera
    GLfloat yaw_;
    // The Pitch (Left/Right) of the Camera
    GLfloat pitch_;
    // Time between current frame and last frame
    GLfloat delta_time_;	
    // Time of last frame
    GLfloat last_frame_;
    
    // MOUSE SETTINGS:
    // Stores previous mouse X location on Window
    int prev_mouse_x_;
    // Stores previous mouse Y location on Window
    int prev_mouse_y_;
    // Left Button Status
    bool is_left_button_;
    // Right Button Status
    bool is_right_button_;
};

// =======================================================================// 
// The following functions are simple accessor/mutator functions          //        
// =======================================================================//  

// Changes the current state of the camera to the specified state
//   @param State, the mode and orientation state of the camera
inline void Camera::ChangeState(State state) {
  state_ = state;
}
// Mutates the cameras position
//   @warn Used to setup default camera, not tested for other use
//   @warn probably shouldn't be inline but compiler will handle it..
inline void Camera::ResetPosition(const glm::vec3 &camera_position) {
  cam_pos_ = camera_position;
  cam_front_.x = 0.0f;
  cam_front_.y = 0.0f;
  cam_front_.z = -1.0f;
  cam_up_.x = 0.0f;
  cam_up_.y = 1.0f;
  cam_up_.z = 0.0f;
}
// Accessor for the camera matrix
inline glm::mat4 Camera::view_matrix() const {
  return view_matrix_;
}
// Accessor for the Current Zoom Aspect Ratio
//  To be used in render
inline float Camera::aspect() const {
  return aspect_;
}
// Accessor for the Current Camera Position
inline glm::vec3 Camera::cam_pos() const {
  return cam_pos_;
}
// Accessor for the Current Camera State
inline Camera::State Camera::state() const {
  return state_;
}
// MOUSE SETTINGS
// Left and Right Button States
inline void Camera::set_is_left_button(const bool &state) {
  is_left_button_ = state;
}
inline void Camera::set_is_right_button(const bool &state) {
  is_right_button_ = state;
}
inline bool Camera::is_left_button() const {
  return is_left_button_;
}
inline bool Camera::is_right_button() const {
  return is_right_button_;
}
// Set both X and Y previous positions of Mouse
inline void Camera::UpdatePreviousMouse(const int &x, const int &y) {
  prev_mouse_x_ = x;
  prev_mouse_y_ = y;
}
// Access X and Y previous positions of Mouse
inline int Camera::prev_mouse_x() const {
  return prev_mouse_x_;
}
inline int Camera::prev_mouse_y() const {
  return prev_mouse_y_;
}

#endif

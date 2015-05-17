#ifndef ASSIGN3_CAMERA_H_
#define ASSIGN3_CAMERA_H_

#include <vector>
#include <string>
#include <cassert>
#include "model_data.h"
#include "model.h"

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

class Camera {
  public:
    // Default Constructor sets starting position of Camera at 0,0,3
    Camera();

    // Moves the cameras by input received
    //   Strafes left/right and moves forward/backward
    //   @param int key, GLUT enum e.g. GLUT_KEY_LEFT
    void Movement(const int &key);
    // Better method using is_key_pressed_hash for multiple inputs
    void Movement();
    // Moves camera position by the x,y,z translation specified
    //   @param translation, the x,y,z amounts to move camera position by
    void Movement(const glm::vec3 &translation);
    // Trues the key hash on key down event
    inline void KeyPressed(const int &key);
    // Falses the key hash on key down event
    inline void KeyReleased(const int &key);
    // Changes Direction by X and Y mouse inputs
    //   Calculates the difference of previous mouse positions
    //   and current to work out the new direction
    //   @param int x, new mouse x position
    //   @param int y, new mouse y position
    //   @warn relies on UpdatePreviousMouse(x,y) to work
    void ChangeDirection(const int &x, const int &y);
    // Changes the direction of the camera to face the given target
    //   @param point, the position vertex to point at
    //   @warn TODO very unoptimized (UpdateCamera should change)
    void ChangeDirection(const glm::vec3 &point);
    // Changes Aspect ratio to Zoom the camera
    //   Calculates the difference of previous mouse y positions
    //   and current to work out the new aspect
    //   @param int y, new mouse y position
    //   @warn relies on UpdatePreviousMouse(x,y) to work
    void ChangeZoom(const int &y);
    // Update the Camera Matrix
    void UpdateCamera();
    // Accessor for the camera matrix
    inline glm::mat4 view_matrix() const;
    // Mutates the cameras position
    inline void ResetPosition(const glm::vec3 &camera_position);
    // Accessor for the Current Zoom Aspect Ratio
    //  To be used in render
    inline float aspect() const;
    // Accessor for the Current Camera Position
    inline glm::vec3 cam_pos() const;

    // MOUSE SETTINGS
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
    // Hash representing Glut Special Keys Pressed
    std::vector<bool> is_key_pressed_hash_;
    // Time between current frame and last frame
    GLfloat delta_time_;	
    // Time of last frame
    GLfloat last_frame_;
    
    // MOUSE SETTINGS
    // Stores previous mouse X location on Window
    int prev_mouse_x_;
    // Stores previous mouse Y location on Window
    int prev_mouse_y_;
    // Left Button Status
    bool is_left_button_;
    // Right Button Status
    bool is_right_button_;
};

inline glm::mat4 Camera::view_matrix() const {
  return view_matrix_;
}
// Trues the key hash on key down event
//   @param a key corresponding to is_key_pressed_hash_
inline void Camera::KeyPressed(const int &key) {
  is_key_pressed_hash_.at(key) = true;
}
// Falses the key hash on key down event
//   @param a key corresponding to is_key_pressed_hash_
inline void Camera::KeyReleased(const int &key) {
  is_key_pressed_hash_.at(key) = false;
}
// Mutates the cameras position
//   @warn Used to setup default camera, not tested for other use
inline void Camera::ResetPosition(const glm::vec3 &camera_position) {
  cam_pos_ = camera_position;
  cam_front_.x = 0.0f;
  cam_front_.y = 0.0f;
  cam_front_.z = -1.0f;
  cam_up_.x = 0.0f;
  cam_up_.y = 1.0f;
  cam_up_.z = 0.0f;
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

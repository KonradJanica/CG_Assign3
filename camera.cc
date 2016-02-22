/**
 * Computer Graphics Assignment 3 -
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * camera.cc, implementation of multiple camera types in openGL
 *
 * This file is an implementation of different camera types in openGL
 * including a chase cam, first person cam and a freeview cam
 *
 * This object is responsible for updating Projection matrix
 * 
 *
 * Refer to the comments describing each function for further detail
 *
 *
 */

#include "camera.h"

// Default Constructor sets starting position of Camera at 0,5,-10 (Above Road)
//   @param shaders, an object pointer containing all active shaders
//   @param window_width, the width of the application window
//   @param window_height, the height of the application window
//   @param fov, the default field of view
Camera::Camera(const Shaders * shaders, 
    const int window_width, const int window_height, const float fov) :
  // Default Vars - VIEWING
  state_(kChase),
  cam_pos_(glm::vec3(0.0f,5.0f,-10.0f)),
  cam_front_(glm::vec3(0.0f, -0.5f, 1.0f)),
  cam_up_(glm::vec3(0.0f, 1.0f, 0.0f)),
  yaw_(kPi/2), pitch_(0.0f),
  // Default Vars - WINDOW
  fov_(fov), width_(window_width), height_(window_height),
  // Pointer to all Shaders
  shaders_(shaders) {
    // Setup view
    view_matrix_ = glm::lookAt(cam_pos_, cam_pos_ + cam_front_, cam_up_);
    // Setup projections
    UpdateProjections();
}

// Changes the current state of the camera, hence changes the viewing
// mode and orientation
//   Is used within Movements and change direction as a flow control
void Camera::CycleState() {
  switch(state_) {
    case kFreeView:
      state_ = kChase;
      break;
    case kChase:
      state_ = kFirstPerson;
      break;
    case kFirstPerson:
      state_ = kFreeView;
      break;
  }
}

// Better method using is_key_pressed_hash for multiple inputs
//   Moves the cameras by input received
//   Strafes left/right and moves forward/backward
//   Uses Delta Time to standardize the speed to FPS
//   @warn relies on member function KeyPressed(int) and KeyReleased(int)
void Camera::Movement(float delta_time, const std::vector<bool> &is_key_pressed_hash) {
    GLfloat cameraSpeed = 0.005f * delta_time;
    if (is_key_pressed_hash.at('j')) {
      cam_pos_ -= glm::normalize(glm::cross(cam_front_, cam_up_)) * cameraSpeed;
    }
    if (is_key_pressed_hash.at('l')) {
      cam_pos_ += glm::normalize(glm::cross(cam_front_, cam_up_)) * cameraSpeed;
    }
    if (is_key_pressed_hash.at('i')) {
      cam_pos_ += cameraSpeed * cam_front_;
    }
    if (is_key_pressed_hash.at('k')) {
      cam_pos_ -= cameraSpeed * cam_front_;
    }
    if (is_key_pressed_hash.at('b')) {
      fov_ += 5.0f;
      printf("FOV = %f\n", fov_);
      if (fov_ > 140.0f)
        fov_ = 30.0f;
      UpdateProjections();
    }
}

// Updates all the currently loaded shaders' projection uniforms
void Camera::UpdateProjections() {
  projection_matrix_ = glm::perspective(fov_, (float)resW_ / (float)resH_, 0.1f, 100.0f);
  ShadersProjectionIterator iter(shaders_);
  for (iter = iter.begin(); iter != iter.end(); ++iter) {
    const Shader * shader = *(iter);
    glUseProgram(shader->Id);
    if (shader->projHandle != -1) // Only update found projections
      glUniformMatrix4fv(shader->projHandle, 1, false, glm::value_ptr(projection_matrix_));
  }
}

// Changes Direction by X and Y mouse inputs
//   Calculates the difference of previous mouse positions
//   and current to work out the new direction
//   Uses Euler angles to calculate the new camera front
//   @param int x, new mouse x position
//   @param int y, new mouse y position
//   @warn relies on UpdatePreviousMouse(x,y) to work
void Camera::ChangeDirection(int x, int y) {
    GLfloat xoffset = x - prev_mouse_x_;
    GLfloat yoffset = prev_mouse_y_ - y;

    GLfloat sensitivity = 0.01;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw_   += xoffset;
    pitch_ += yoffset;

    if(pitch_ > k89DegreesToRadians)
      pitch_ = k89DegreesToRadians;
    if(pitch_ < -k89DegreesToRadians)
      pitch_ = -k89DegreesToRadians;

    glm::vec3 front;
    front.x = cos(yaw_) * cos(pitch_);
    front.y = sin(pitch_);
    front.z = sin(yaw_) * cos(pitch_);
    cam_front_ = glm::normalize(front);
}

// Changes FOV to Zoom the camera
//  Calculates the difference of previous mouse y positions
//  and current to work out the new aspect
//  @param int y, new mouse y position
//  @warn relies on UpdatePreviousMouse(x,y) to work
void Camera::ChangeZoom(const int &y) {
  GLfloat yoffset = prev_mouse_y_ - y;
  if(fov_ >= 1.0f && fov_ <= 45.0f)
    fov_ -= yoffset;
  if(fov_ <= 1.0f)
    fov_ = 1.0f;
  if(fov_ >= 45.0f)
    fov_ = 45.0f;
}

// Update upon Car movement tick
//   Called in Car's physics tick
void Camera::UpdateCarTick(const Car * car) {
  // Follow car in Chase and FirstPerson mode
  // Only lock view to car in Chase mode
  // Only lock position to car in FirstPerson mode
  switch(state_) {
    case kChase:
    {
      cam_front_ = car->translation();

      glm::vec2 dir_raw = glm::vec2(-car->velocity_x(), -car->velocity_z());
      dir_raw = glm::normalize(dir_raw);
      glm::vec3 direction = glm::vec3(dir_raw.x, 0.5f, dir_raw.y);

      cam_pos_ = car->translation();
      cam_pos_ += 6.5f*direction;
      cam_pos_ += car->displacement();
      break;
    }
    case kFirstPerson:
    {
      glm::vec3 direction;
      direction.x = car->direction().x;
      direction.y = 0;
      direction.z = car->direction().z;
      cam_front_ = direction;
      cam_pos_ = car->translation();
      cam_pos_.x += -0.08f * direction.x;
      cam_pos_.z += -0.08f * direction.z;
      cam_pos_.y += 0.28f;
      break;
    }
    case kFreeView:
    //do nothing
    break;
  }
}

// Update the VIEW Matrix
void Camera::UpdateCamera() {
  if (state_ == kFreeView || state_ == kFirstPerson) {
    view_matrix_ = glm::lookAt(cam_pos_, cam_pos_ + cam_front_, cam_up_);
  } else {
    view_matrix_ = glm::lookAt(cam_pos_, cam_front_, cam_up_);
  }
}

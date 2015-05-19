#include "object.h"

// Construct with position setting parameters
Object::Object(const glm::vec3 &translation,
               const glm::vec3 &rotation,
               const glm::vec3 &scale,
               float default_velocity)
  : translation_(translation), rotation_(rotation), scale_(scale),
    displacement_(0), velocity_(default_velocity) {
    UpdateModelMatrix();
  }

// Updates the all the movement data for the object
// @warn should be called in controller tick
void Object::ControllerMovementTick(float delta_time, const std::vector<bool> &is_key_pressed_hash) {
  float MOVESPEED = 0.004f * delta_time;
  float TURNRATE = 0.2f * delta_time;

  // Used to update camera
  displacement_ = glm::vec3(0,0,0);

  if (is_key_pressed_hash.at('w')) {
    velocity_ += 0.00001f * delta_time;
  }
  if (is_key_pressed_hash.at('s')) {
    velocity_ += -0.00005f * delta_time;
  }

  if (velocity() > 0) {
    if (is_key_pressed_hash.at('a')) {
      float rot = TURNRATE * 0.005f/velocity_;
      if (rot > TURNRATE)
        rot = TURNRATE;
      set_rotation(glm::vec3(rotation().x, rotation().y + rot, rotation().z));
    }
    if (is_key_pressed_hash.at('d')) {
      float rot = TURNRATE * 0.005f/velocity_;
      if (rot > TURNRATE)
        rot = TURNRATE;
      set_rotation(glm::vec3(rotation().x, rotation().y - rot, rotation().z));
    }
  }
}

// Sets the acceleration of the object to given amount
// TODO proper comment
void Object::CalcPosition() {
  float x_movement = velocity_ * sin(DEG2RAD(rotation().y));
  float z_movement = velocity_ * cos(DEG2RAD(rotation().y));
  translation_.x += x_movement;
  translation_.z += z_movement;
  displacement_.x += x_movement;
  displacement_.z += z_movement;
  velocity_ -= 0.000001f;
  if (velocity_ < 0)
    velocity_ = 0;
}

// Updates the transform matrix using glLookAt
//  Includes physics calulations and movements if they exist
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateModelMatrix() {
  CalcPosition();

  // Scale of object
  glm::mat4 scale = glm::scale(  glm::mat4(1.0f), 
      glm::vec3(scale_.x, scale_.y, scale_.z));

  // Rotation of object
  glm::mat4 rotate = glm::mat4(1.0f);
  rotate = glm::rotate(rotate, rotation_.x, glm::vec3(1, 0, 0));
  rotate = glm::rotate(rotate, rotation_.y, glm::vec3(0, 1, 0));
  rotate = glm::rotate(rotate, rotation_.z, glm::vec3(0, 0, 1));

  // Translation of object
  glm::mat4 translate = glm::translate(  glm::mat4(1.0f), 
      glm::vec3(translation_.x, translation_.y, translation_.z));

  model_matrix_ = translate * rotate * scale;
}

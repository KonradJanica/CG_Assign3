#include "object.h"

// Construct with position setting parameters
Object::Object(const glm::vec3 &translation,
               const glm::vec3 &rotation,
               const glm::vec3 &scale,
               float default_speed)
  : translation_(translation), rotation_(rotation), scale_(scale),
    displacement_(0), speed_(default_speed) {
    UpdateModelMatrix();
  }

// Updates the all the movement data for the object
// @warn should be called in controller tick
void Object::ControllerMovementTick(float delta_time, const std::vector<bool> &is_key_pressed_hash) {
  // Convert delta_time to ticks per second
  //   Currently ticks per milisecond
  delta_time /= 1000;
  // TODO put into separate constants class
  float TURNRATE = 100 * delta_time;
  float MASS = 1500; //kg
  float ENGINEFORCE = 9000; //newtons
  float BRAKINGFORCE = ENGINEFORCE * 5; //newtons
  float AIRRESSISTANCE = 0.4257;  //proportional constant
  float FRICTION = AIRRESSISTANCE * 30;
  float SPEEDSCALE = 5; //the conversions from real speed to game movement

  // SETUP VARS
  // Used to update camera
  displacement_ = glm::vec3(0,0,0);
  // The current force added to velocity
  float force_x = 0;
  float force_z = 0;
  // The current velocity vector
  float direction_x = sin(DEG2RAD(rotation().y));
  float direction_z = cos(DEG2RAD(rotation().y));
  float velocity_x = speed_ * direction_x;
  float velocity_z = speed_ * direction_z;

  if (speed() > 0) {
    if (is_key_pressed_hash.at('a')) {
      float rot = TURNRATE * 20/speed_;
      if (rot > TURNRATE)
        rot = TURNRATE;
      set_rotation(glm::vec3(rotation().x, rotation().y + rot, rotation().z));
    }
    if (is_key_pressed_hash.at('d')) {
      float rot = TURNRATE * 20/speed_;
      if (rot > TURNRATE)
        rot = TURNRATE;
      set_rotation(glm::vec3(rotation().x, rotation().y - rot, rotation().z));
    }
  }


  if (is_key_pressed_hash.at('w')) {
    force_x += ENGINEFORCE * direction_x;
    force_z += ENGINEFORCE * direction_z;
    // TODO switch gear shift different accelerations
  }
  if (is_key_pressed_hash.at('s') && speed() > 0) {
    force_x -= BRAKINGFORCE * direction_x;
    force_z -= BRAKINGFORCE * direction_z;
  }

  // Rolling resistance (friction of tires)
  force_x -= FRICTION * velocity_x * speed_;
  force_z -= FRICTION * velocity_z * speed_;
  // Air resistance x
  force_x -= AIRRESSISTANCE * speed_;
  force_z -= AIRRESSISTANCE * speed_;

  // CALCULATE ACCELERATION => a = F/M
  float acceleration_x = force_x / MASS;
  float acceleration_z = force_z / MASS;

  // CALCULATE VELOCITY => v = v+dt*a 
  velocity_x += delta_time * acceleration_x;
  velocity_z += delta_time * acceleration_z;

  // CALCULATE SPEED
  if (velocity_x < 0 && velocity_z < 0) {
    speed_ = 0;
  } else {
    speed_ = sqrt(velocity_x * velocity_x + velocity_z * velocity_z);
  }
  printf("speed = %f\n", speed_);
  // convert speed to game world speed
  // TODO put into separate constants class
  velocity_x /= SPEEDSCALE;
  velocity_z /= SPEEDSCALE;

  // CALCULATE NEW POSITION => p = p+dt*v
  translation_.x += delta_time * velocity_x;
  translation_.z += delta_time * velocity_z;
  displacement_.x += delta_time * velocity_x;
  displacement_.z += delta_time * velocity_z;
}

// Updates the transform matrix using glLookAt
//  Includes physics calulations and movements if they exist
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateModelMatrix() {

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

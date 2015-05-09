#include "object.h"

// Default constructor make identity transform with no scaling
Object::Object() : position_(glm::vec3(0,0,0)), direction_(glm::vec3(0,0,1)), up_(glm::vec3(0,1,0)), scale_(glm::vec3(1,1,1)), physics_extension_(0) {
  transform_ = glm::mat4(1); //Identity matrix
}

// Construct with position setting parameters
Object::Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up, const glm::vec3 &scale)
  : position_(position), direction_(direction), up_(up), scale_(scale), physics_extension_(0) {
    UpdateTransform();
  }

// Enables the physics extension
//  @warn Is created on heap, needs to be deleted afterwards
void Object::EnablePhysics(const float &velocity, const float &acceleration, const float &turn_rate) {
  if (IsPhysics()) {
    printf("Physics being overwritten, possible memory leak\n");
  }
  physics_extension_ = new Physics(velocity, acceleration, turn_rate);
}

// Tests for whether Physics are enabled for the object
bool Object::IsPhysics() const {
  return physics_extension_;
}

// Increases the acceleration of the object by given amount
//   TODO a maximum acceleration?
void Object::Accelerate(const float &amount) {
  physics_extension_->acceleration += amount;
}

// Decreases the acceleration of the object by given amount
//   TODO a minimum acceleration?
void Object::Deccelerate(const float &amount) {
  physics_extension_->acceleration -= amount;
}

// Updates the transform matrix using glLookAt
//  Includes physics calulations and movements if they exist
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateTransform() {
  if (IsPhysics()) {
    // Calculate Delta Time to even out for different frame rates
    GLfloat current_frame = glutGet(GLUT_ELAPSED_TIME);
    GLfloat &delta_time = physics_extension_->delta_time;
    GLfloat &last_frame = physics_extension_->last_frame;
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
    // Calculate Physics
    physics_extension_->velocity += physics_extension_->acceleration;
    position_ += physics_extension_->velocity * direction_;
  }
  transform_ = glm::lookAt(position_, position_+ direction_, up_);
  transform_ = glm::scale(transform_, scale_);
}

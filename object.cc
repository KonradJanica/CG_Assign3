#include "object.h"

// Default constructor make identity transform with no scaling
Object::Object() : position_(glm::vec3(0,0,0)), direction_(glm::vec3(0,0,1)), up_(glm::vec3(0,1,0)), scale_(glm::vec3(1,1,1)) {
  transform_ = glm::mat4(1); //Identity matrix
}

// Construct with position setting parameters
Object::Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up, const glm::vec3 &scale)
  : position_(position), direction_(direction), up_(up), scale_(scale) {
    UpdateTransform();
}

// Updates the transform matrix using glLookAt
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateTransform() {
  transform_ = glm::lookAt(position_, position_+ direction_, up_);
  transform_ = glm::scale(transform_, scale_);
}

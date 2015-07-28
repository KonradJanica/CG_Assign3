#include "object.h"

// Construct with position setting parameters
Object::Object(const Shader &shader,
               const std::string &model_filename,
               const glm::vec3 &translation,
               const glm::vec3 &rotation,
               const glm::vec3 &scale,
               float default_speed, bool debugging_on)
  : Model(shader, model_filename,
      translation, rotation, scale,
      default_speed, debugging_on),
  kDefaultHeight(translation.y),
  translation_(translation), rotation_(rotation), scale_(scale)
  {
    UpdateModelMatrix();
  }

// Updates the transform matrix using glLookAt
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateModelMatrix() {

  // Scale of object
  glm::mat4 scale = glm::scale(  glm::mat4(1.0f), 
      glm::vec3(scale_.x, scale_.y, scale_.z));

  // Rotation of object - @warn order of rotation matters
  glm::mat4 rotate = glm::mat4(1.0f);
  rotate = glm::rotate(rotate, rotation_.y, glm::vec3(0, 1, 0));
  rotate = glm::rotate(rotate, rotation_.x, glm::vec3(1, 0, 0));
  rotate = glm::rotate(rotate, rotation_.z, glm::vec3(0, 0, 1));

  // Translation of object
  glm::mat4 translate = glm::translate(  glm::mat4(1.0f), 
      glm::vec3(translation_.x, translation_.y, translation_.z));

  model_matrix_ = translate * rotate * scale;
}

// Accessor for the direction vector
//   @warn the roll (z rotation) is not calculated
glm::vec3 Object::direction() const {
  glm::vec3 direction = glm::vec3(
      sin(DEG2RAD(rotation().y)),  //dir.x
      -sin(DEG2RAD(rotation().x)), //dir.y
      cos(DEG2RAD(rotation().y))); //dir.z
  direction = glm::normalize(direction);
  return direction;
}

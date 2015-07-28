#include "car.h"

// Construct with position setting parameters
Car::Car(const Shader &shader,
         const std::string &model_filename,
         const glm::vec3 &translation,
         const glm::vec3 &rotation,
         const glm::vec3 &scale,
         float default_speed, bool debugging_on)
  : Object(shader, model_filename,
      translation,
      rotation,
      scale,
      default_speed, debugging_on)
{}

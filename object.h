#ifndef ASSIGN3_OBJECT_H_
#define ASSIGN3_OBJECT_H_

#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>
#include "model.h"
#include "shaders/shaders.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// TODO put into separate constants class
#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

// Object is a Model with model_matrix transformations
//  Adds extra methods and members to control matrix transformations
class Object : public Model {
  public:
    const float kDefaultHeight;

    // Construct with position setting parameters
    //   The Y translation becomes kDefaultHeight
    Object(const Shader &shader,
           const std::string &model_filename,
           const glm::vec3 &translation,
           const glm::vec3 &rotation,
           const glm::vec3 &scale,
           float default_speed = 0,
           bool debugging_on = false);

    // Updates the model matrix using glLookAt
    //  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
    void UpdateModelMatrix();
    // Calculate the direction vector
    //   @warn the roll (z rotation) is not calculated
    glm::vec3 direction() const;

    // ACCESSORS:
    // Accessor for the current model matrix
    inline glm::mat4 model_matrix() const;
    // Accessor for the position vector
    inline glm::vec3 translation() const;
    // Accessor for the rotation vector
    inline glm::vec3 rotation() const;

    // MUTATORS:
    // Sets the position
    //   @warn requires a call to UpdateModelMatrix() afterwards
    inline void set_translation(glm::vec3 new_translation);
    // Sets the direction
    //   @warn requires a call to UpdateModelMatrix() afterwards
    inline void set_rotation(glm::vec3 new_rotation);


  private:
    // Model matrix assosciated with object
    //   Use UpdateModelMatrix() to update
    //   @warn Needs to be updated everytime glLookAt vectors (below) are changed
    glm::mat4 model_matrix_;

    // World transformations
    // The rotation the object is facing
    glm::vec3 rotation_;
    // The direction of the object
    //   @warn roll is not calculated
    glm::vec3 direction_;

    // glm::scale transformation
    //   The scale of the object
    const glm::vec3 scale_;

    // Add a wireframe model from .obj file to the scene
    void AddModel(GLuint program_id, const std::string &model_filename);

  protected:
    // World transformations
    // The position of the object in the world
    glm::vec3 translation_;
    
};

// ACCESSORS:
// Accessor for the current model matrix
inline glm::mat4 Object::model_matrix() const {
  return model_matrix_;
}
// Accessor for the position vector
inline glm::vec3 Object::translation() const {
  return translation_;
}
// Accessor for the direction vector
inline glm::vec3 Object::rotation() const {
  return rotation_;
}

// MUTATORS:
// Sets the position
//   @warn requires a call to UpdateModelMatrix() afterwards
inline void Object::set_translation(glm::vec3 new_translation) {
  translation_ = new_translation;
}
// Sets the direction
//   @warn requires a call to UpdateModelMatrix() afterwards
inline void Object::set_rotation(glm::vec3 new_rotation) {
  rotation_ = new_rotation;
}

#endif

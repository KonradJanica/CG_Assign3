#ifndef ASSIGN2_OBJECT_H_
#define ASSIGN2_OBJECT_H_

#include <vector>
#include <string>
#include <cassert>
#include "model_data.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Object call contains a model by inhertiance
//  Adds extra methods and members to control matrix transformations
//  @usage Object * car = new model(program_id, "car-n.obj")
class Object {
  public:
    // Default constructor make identity transform with no scaling
    Object();
    // Construct with position setting parameters
    Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up, const glm::vec3 &scale = glm::vec3(1,1,1));

    // Updates the transform matrix using glLookAt
    //  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
    void UpdateTransform();
    ////////////
    // ACCESSORS
    // Accessor for the current transformation matrix
    inline glm::mat4 transform() const;
    // Accessor for the position vector
    inline glm::vec3 position() const;
    // Accessor for the direction vector
    inline glm::vec3 direction() const;
    // Accessor for the up vector
    inline glm::vec3 up() const;
    ///////////
    // MUTATORS
    // Sets the position
    //   @warn requires a call to UpdateTransform() afterwards
    inline void set_position(glm::vec3 new_position);
    // Sets the direction
    //   @warn requires a call to UpdateTransform() afterwards
    inline void set_direction(glm::vec3 new_direction);
    // Sets the up
    //   @warn requires a call to UpdateTransform() afterwards
    inline void set_up(glm::vec3 new_up);

  private:
    // Transformation matrix Assosciated with object
    //  Use UpdateTransform() to update
    //  @warn Needs to be updated everytime glLookAt vectors (below) are changed
    glm::mat4 transform_;

    /////////////////
    // glLookAt transformations
    // The position of the object in the world
    glm::vec3 position_;
    // The direction the object is facing
    glm::vec3 direction_;
    // The up vector of the object
    glm::vec3 up_;

    /////////////////
    // glm::scale transformation
    // The scale of the object
    glm::vec3 scale_;


    // Add a wireframe model from .obj file to the scene
    void AddModel(GLuint &program_id, const std::string &model_filename);
};

////////////
// ACCESSORS
// Accessor for the current transformation matrix
inline glm::mat4 Object::transform() const {
  return transform_;
}
// Accessor for the position vector
inline glm::vec3 Object::position() const {
  return position_;
}
// Accessor for the direction vector
inline glm::vec3 Object::direction() const {
  return direction_;
}
// Accessor for the up vector
inline glm::vec3 Object::up() const {
  return up_;
}
///////////
// MUTATORS
// Sets the position
//   @warn requires a call to UpdateTransform() afterwards
inline void Object::set_position(glm::vec3 new_position) {
  position_ = new_position;
}
// Sets the direction
//   @warn requires a call to UpdateTransform() afterwards
inline void Object::set_direction(glm::vec3 new_direction) {
  direction_ = new_direction;
}
// Sets the up
//   @warn requires a call to UpdateTransform() afterwards
inline void Object::set_up(glm::vec3 new_up) {
  up_ = new_up;
}
#endif

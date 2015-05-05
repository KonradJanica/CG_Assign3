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

class Object {
  public:
    // Enum for vertex coordinates
    //   used in GetMax
    enum CoordEnum {
      kX = 0,
      kY = 1, 
      kZ = 2,
      kx = 0, 
      ky = 1, 
      kz = 2,
      kMin = 3,
      kMax = 3,
    };  

    // Construct with parameters required to construct model
    Object(const GLuint &program_id, const std::string &model_filename);
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
    // Model Assosciated with Object
    Model * model_;
    // Transformation matrix Assosciated with object
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
inline glm::mat4 transform() const {
  return transform_;
}
// Accessor for the position vector
inline glm::vec3 position() const {
  return position_;
}
// Accessor for the direction vector
inline glm::vec3 direction() const {
  return direction_;
}
// Accessor for the up vector
inline glm::vec3 up() const {
  return up_;
}
///////////
// MUTATORS
// Sets the position
//   @warn requires a call to UpdateTransform() afterwards
inline void set_position(glm::vec3 new_position) {
  position_ = new_position;
}
// Sets the direction
//   @warn requires a call to UpdateTransform() afterwards
inline void set_direction(glm::vec3 new_direction) {
  direction_ = new_direction;
}
// Sets the up
//   @warn requires a call to UpdateTransform() afterwards
inline void set_up(glm::vec3 new_up) {
  up_ = new_up;
}
#endif

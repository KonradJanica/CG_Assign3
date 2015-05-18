#ifndef ASSIGN3_OBJECT_H_
#define ASSIGN3_OBJECT_H_

#include <vector>
#include <string>
#include <cassert>

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "lib/shader/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Object call contains a model by inhertiance
//  Adds extra methods and members to control matrix transformations
//  Can also hold physics information if it is a moving object
//  Physics are enabled if Physics * physics_extension_ != 0
//  @usage Object * car = new model(program_id, "car-n.obj")
class Object {
  public:
    // The data required to move an object with physics
    //   Data is updated in UpdateModelMatrix()
    struct Physics {
      // The movement of the object in it's direction
      float velocity;
      // The amount to add to velocity each tick
      float acceleration;
      // The turn speed of the object
      //  TODO should be added to direction with input left/right
      float turn_rate;
      // Used for evening out over different frame rates
      GLfloat delta_time;
      // Used for evening out over different frame rates
      GLfloat last_frame;
      Physics(const float &v, const float &a, const float &t)
        : velocity(v), acceleration(a), turn_rate(t),
        delta_time(0), last_frame(0) {};
    };

    // Construct with position setting parameters
    Object(const glm::vec3 &translation, const glm::vec3 &rotation = glm::vec3(0.0f,0.0f,0.0f), const glm::vec3 &scale = glm::vec3(1,1,1));

    // Updates the model matrix using glLookAt
    //  Includes physics calulations and movements if they exist
    //  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
    void UpdateModelMatrix();

    // ACCESSORS:
    // Accessor for the current model matrix
    inline glm::mat4 model_matrix() const;
    // Accessor for the position vector
    inline glm::vec3 translation() const;
    // Accessor for the rotation vector
    inline glm::vec3 rotation() const;
    // Accessor for the displacement vector
    inline glm::vec3 displacement() const;

    // MUTATORS:
    // Sets the position
    //   @warn requires a call to UpdateModelMatrix() afterwards
    inline void set_translation(glm::vec3 new_translation);
    // Sets the direction
    //   @warn requires a call to UpdateModelMatrix() afterwards
    inline void set_rotation(glm::vec3 new_rotation);
    // Sets the displacement
    inline void set_displacement(glm::vec3 new_displacement);
    
    // PHYSICS:
    // Enables the physics extension
    //  @warn Is created on heap, needs to be deleted afterwards
    void EnablePhysics(const float &velocity, const float &acceleration, const float &turn_rate);
    // Tests for whether Physics are enabled for the object
    bool IsPhysics() const;
    // Increases the acceleration of the object by given amount
    //   TODO a maximum acceleration?
    void Accelerate(const float &amount);
    // Decreases the acceleration of the object by given amount
    //   TODO a minimum acceleration?
    void Deccelerate(const float &amount);

    // VIRTUAL CHILD (Model) METHODS:
    // Accessor for current shader program.
    //   @return program_id_, the shader used by the model
    virtual GLuint program_id() const = 0;
    // Accessor for each shapes VAO and it's corresponding texture
    //   Each VAO includes indices, vertices and UV coordinates
    //   @return vao_texture_handle_, a container for all VAOs and their corresponding textures
    virtual std::vector<std::pair<unsigned int, GLuint> > vao_texture_handle() const = 0;
    // Accessor for largest vertex
    //   @param enum value
    //   @return max_$_, the maximum cartesian coordinate of given input
    virtual float GetMax( int e_numb ) const = 0;
    // Accessor for smallest vertex
    //   @param enum value
    //   @return min_$_, the minimum cartesian coordinate of given input
    virtual float GetMin( int e_numb ) const = 0;
    // Accessor for the points of a given VAO shape
    //   @param index of shape
    //   @return unsigned int, amount of points in the VAO shape
    //   @warn throws exception on error
    virtual unsigned int points_per_shape_at(unsigned int x) const = 0;
    // Accessor for the Ambient Surface Colours vector
    //   @param index of colour
    //   @return ambient_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    virtual glm::vec3 ambient_surface_colours_at(unsigned int index) const = 0;
    // Accessor for the Diffuse Surface Colours vector
    //   @param index of colour
    //   @return diffuse_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    virtual glm::vec3 diffuse_surface_colours_at(unsigned int index) const = 0;
    // Accessor for the Specular Surface Colours vector
    //   @param index of colour
    //   @return glm::vec3, a vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    virtual glm::vec3 specular_surface_colours_at(unsigned int index) const = 0;
    // Accessor for the Shininess vector
    //   @param index of shininess
    //   @return float, a float corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    virtual float shininess_at(unsigned int index) const = 0;
    // Accessor for all of the points in the shape
    //   @return amount_points_, the total amount of points => used for rendering
    virtual unsigned int amount_points() const = 0;

    virtual GLuint skytexture(int x) const = 0;
    virtual unsigned int skyboxvao() const = 0;


  private:
    // Model matrix assosciated with object
    //   Use UpdateModelMatrix() to update
    //   @warn Needs to be updated everytime glLookAt vectors (below) are changed
    glm::mat4 model_matrix_;

    // World transformations
    // The position of the object in the world
    glm::vec3 translation_;
    // The rotation the object is facing
    glm::vec3 rotation_;
    // The amount moved from last tick
    glm::vec3 displacement_;

    // glm::scale transformation
    //   The scale of the object
    glm::vec3 scale_;

    // The physics extension for moving objects
    //   Will be 0 (null_ptr) if doesn't exist
    Physics * physics_extension_;

    // Add a wireframe model from .obj file to the scene
    void AddModel(GLuint &program_id, const std::string &model_filename);
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
// Accessor for the displacement vector
inline glm::vec3 Object::displacement() const {
  return displacement_;
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
// Sets the displacement
inline void Object::set_displacement(glm::vec3 new_displacement) {
  displacement_ = new_displacement;
}
#endif

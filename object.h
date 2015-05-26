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

// TODO put into separate constants class
#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

// Object call contains a model by inhertiance
//  Adds extra methods and members to control matrix transformations
//  Can also hold physics information if it is a moving object
//  Physics are enabled if Physics * physics_extension_ != 0
//  @usage Object * car = new model(program_id, "car-n.obj")
class Object {
  public:
    // Construct with position setting parameters
    Object(const glm::vec3 &translation, 
           const glm::vec3 &rotation = glm::vec3(0.0f,0.0f,0.0f), 
           const glm::vec3 &scale = glm::vec3(1,1,1),
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
    // Accessor for the displacement vector
    inline glm::vec3 displacement() const;
    // Accessor for the centripetal X velocity
    inline float centripeta_velocity_x() const;
    // Accessor for the centripetal Z velocity
    inline float centripeta_velocity_z() const;

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
    // Updates the all the movement data for the object
    // @warn should be called in controller tick
    void ControllerMovementTick(float delta_time, const std::vector<bool> &is_key_pressed_hash);
    // Sets the acceleration of the object to given amount
    // TODO comment
    void CalcPosition();
    // Accessor for the current speed of the object
    //   @return speed_, the current speed of the object
    inline float speed() const;
    // Accessor for the constant default speed of the object
    //   @return default_speed_, the default speed of the object
    inline float default_speed() const;
    // Reset speed and acceleration to 0
    inline void ResetPhysics();
    // Reduce speed by amount
    //   Corrects speed < 0 to 0
    //   @param the amount to decrease the speed by
    inline void ReduceSpeed(float amount);

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
    // The direction of the object
    //   @warn roll is not calculated
    glm::vec3 direction_;

    // glm::scale transformation
    //   The scale of the object
    glm::vec3 scale_;

    // The amount moved from last tick
    glm::vec3 displacement_;

    // The speed of the object in the direction it is facing
    float speed_;
    // The default speed of the object
    const float default_speed_;
    // The speed of the object at 90 degrees towards turning center
    float centri_speed_;
    // The centripetal velocitites of the object
    float centripeta_velocity_x_;
    float centripeta_velocity_z_;

    // Verbose debugging - prints physics variables
    bool is_debugging_;

    // Works out the maximum speed achieveable per gear
    //   @param  the gear ratio
    //   @return  the max speed of given gear ratio
    float MaxSpeedPerGear(float g_num);

    // Works out the maximum force per gear
    //   @param  the gear ratio
    //   @return  the maximum force per gear
    float MaxEngineForcePerGear(float g_num, float max_torque);

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
// Accessor for the current speed of the object
//   @return speed_, the current speed of the object
inline float Object::speed() const {
  return speed_;
}
// Accessor for the constant default speed of the object
//   @return default_speed_, the default speed of the object
inline float Object::default_speed() const {
  return default_speed_;
}
// Accessor for the centripetal X velocity
inline float Object::centripeta_velocity_x() const {
  return centripeta_velocity_x_;
}
// Accessor for the centripetal Z velocity
inline float Object::centripeta_velocity_z() const {
  return centripeta_velocity_z_;
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
// Reset speed and acceleration to 0
inline void Object::ResetPhysics() {
  speed_ = default_speed();
  centri_speed_ = 0;
}
// Reduce speed by amount
//   Corrects speed < 0 to 0
//   @param the amount to decrease the speed by
inline void Object::ReduceSpeed(float amount) {
  speed_ -= amount;
  if (speed() < 0) {
    speed_ = 0;
  }
}

#endif

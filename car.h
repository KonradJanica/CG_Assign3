#ifndef ASSIGN3_CAR_H_
#define ASSIGN3_CAR_H_

#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>
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

// Car contains a model and object by inhertiance
//  Adds extra methods and members to control matrix transformations
//  Can also hold physics information if it is a moving object
//  @usage Car * car = new object(program_id, "car-n.obj")
class Car {
  public:
    // Construct with position setting parameters
    //   The Y translation becomes kDefaultHeight
    Car(const glm::vec3 &translation,
           const glm::vec3 &rotation,
           const glm::vec3 &scale,
           float default_speed = 0,
           bool debugging_on = false);

    // Updates the model matrix using glLookAt
    //  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
    virtual void UpdateModelMatrix() = 0;
    // Calculate the direction vector
    //   @warn the roll (z rotation) is not calculated
    virtual glm::vec3 direction() const = 0;

    // ACCESSORS:
    // Accessor for the current model matrix
    virtual glm::mat4 model_matrix() const = 0;
    // Accessor for the position vector
    virtual glm::vec3 translation() const = 0;
    // Accessor for the rotation vector
    virtual glm::vec3 rotation() const = 0;
    // Accessor for the displacement vector
    virtual glm::vec3 displacement() const = 0;
    // Accessor for the centripetal X velocity
    virtual float centripeta_velocity_x() const = 0;
    // Accessor for the centripetal Z velocity
    virtual float centripeta_velocity_z() const = 0;
    // Accessor for the total X velocity
    virtual float velocity_x() const = 0;
    // Accessor for the total Z velocity
    virtual float velocity_z() const = 0;

    // MUTATORS:
    // Sets the position
    //   @warn requires a call to UpdateModelMatrix() afterwards
    virtual void set_translation(glm::vec3 new_translation) = 0;
    // Sets the direction
    //   @warn requires a call to UpdateModelMatrix() afterwards
    virtual void set_rotation(glm::vec3 new_rotation) = 0;
    // Sets the displacement
    virtual void set_displacement(glm::vec3 new_displacement) = 0;

    // PHYSICS:
    // Updates the all the movement data for the object
    // @warn should be called in controller tick
    virtual void ControllerMovementTick(float delta_time, const std::vector<bool> &is_key_pressed_hash) = 0;
    // Sets the acceleration of the object to given amount
    // TODO comment
    virtual void CalcPosition() = 0;
    // Accessor for the current speed of the object
    //   @return speed_, the current speed of the object
    virtual float speed() const = 0;
    // Accessor for the constant default speed of the object
    //   @return default_speed_, the default speed of the object
    virtual float default_speed() const = 0;
    // Reset speed and acceleration to 0
    virtual void ResetPhysics() = 0;
    // Reduce speed by amount
    //   Corrects speed < 0 to 0
    //   @param the amount to decrease the speed by
    virtual void ReduceSpeed(float amount) = 0;
    // Reduce centripetal velocity by an amount
    //   @param percentage amount
    //   @warn should only be called when car is not in driving mode
    virtual void ReduceCentriVelocity(float percentage) = 0;
    // VIRTUAL CHILD (Model) METHODS:
    // Accessor for current shader program.
    //   @return program_id_, the shader used by the model
    virtual const Shader * shader() const = 0;
    // Accessor for each shapes VAO and it's corresponding texture
    //   Each VAO includes indices, vertices and UV coordinates
    //   @return vao_texture_handle_, a container for all VAOs and their corresponding textures
    virtual const std::vector<std::pair<unsigned int, GLuint> > * vao_texture_handle() const = 0;
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
    // Accessor for the Dissolve vector
    //   @param index of dissolve vector
    //   @return float, a float corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    virtual float dissolve_at(unsigned int index) const = 0;
    // Accessor for all of the points in the shape
    //   @return amount_points_, the total amount of points => used for rendering
    virtual unsigned int amount_points() const = 0;

  private:

};


#endif

#ifndef ASSIGN3_CAR_H_
#define ASSIGN3_CAR_H_

#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>
#include "shaders/shaders.h"
#include "object.h"

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
class Car : public Object {
  public:
    // Construct with position setting parameters
    //   The Y translation becomes kDefaultHeight
    Car(const Shader &shader,
        const std::string &model_filename,
        const glm::vec3 &translation,
        const glm::vec3 &rotation,
        const glm::vec3 &scale,
        float default_speed = 0,
        bool debugging_on = false);

    // ACCESSORS
    // Accessor for the displacement vector
    inline glm::vec3 displacement() const;
    // Accessor for the centripetal X velocity
    inline float centripeta_velocity_x() const;
    // Accessor for the centripetal Z velocity
    inline float centripeta_velocity_z() const;
    // Accessor for the total X velocity
    inline float velocity_x() const;
    // Accessor for the total Z velocity
    inline float velocity_z() const;

    // PHYSICS:
    // Updates all the movement data for the PLAYER object
    // @warn should be called in controller tick
    void ControllerMovementTick(float delta_time, const std::vector<bool> &is_key_pressed_hash);
    // Updates all the movement data for the NPC object
    // @warn should be called in controller tick
    void ControllerMovementTick(float delta_time);
    // Accessor for the current speed of the object
    //   @return speed_, the current speed of the object
    inline float speed() const;
    // Accessor for the constant default speed of the object
    //   @return default_speed_, the default speed of the object
    inline float default_speed() const;
    // Accessor for the constant default height of the object
    //   @return default_height_, the default height of the object (above road)
    inline float default_height() const;
    // Reset speed and acceleration to 0
    inline void ResetPhysics();
    // Reduce speed by amount
    //   Corrects speed < 0 to 0
    //   @param the amount to decrease the speed by
    inline void ReduceSpeed(float amount);
    // Reduce centripetal velocity by an amount
    //   @param percentage amount
    //   @warn should only be called when car is not in driving mode
    inline void ReduceCentriVelocity(float percentage);

    // MUTATORS
    // Sets the displacement
    inline void set_displacement(glm::vec3 new_displacement);

  private:
    // The amount moved from last tick
    glm::vec3 displacement_;

    // The speed of the object in the direction it is facing
    float speed_;
    // The centripetal speed of the object (for optimizing roll)
    float centri_speed_;
    // The default speed of the object
    const float default_speed_;
    // The default height of the object
    const float default_height_;
    // The centripetal velocitites of the object
    float centripeta_velocity_x_;
    float centripeta_velocity_z_;
    // The velocities of the object
    float velocity_x_;
    float velocity_z_;

    // Works out the maximum speed achieveable per gear
    //   @param  the gear ratio
    //   @return  the max speed of given gear ratio
    float MaxSpeedPerGear(float g_num);

    // Works out the maximum force per gear
    //   @param  the gear ratio
    //   @return  the maximum force per gear
    float MaxEngineForcePerGear(float g_num, float max_torque);

    // Verbose debugging - prints physics variables
    const bool is_debugging_;

};

// ACCESSORS
// Accessor for the displacement vector
inline glm::vec3 Car::displacement() const {
  return displacement_;
}
// Accessor for the current speed of the object
//   @return speed_, the current speed of the object
inline float Car::speed() const {
  return speed_;
}
// Accessor for the constant default speed of the object
//   @return default_speed_, the default speed of the object
inline float Car::default_speed() const {
  return default_speed_;
}
// Accessor for the constant default height of the object
//   @return default_height_, the default height of the object (above road)
inline float Car::default_height() const {
  return default_height_;
}
// Accessor for the centripetal X velocity
inline float Car::centripeta_velocity_x() const {
  return centripeta_velocity_x_;
}
// Accessor for the centripetal Z velocity
inline float Car::centripeta_velocity_z() const {
  return centripeta_velocity_z_;
}
// Accessor for the total X velocity
inline float Car::velocity_x() const {
  return velocity_x_;
}
// Accessor for the total Z velocity
inline float Car::velocity_z() const {
  return velocity_z_;
}

// PHYSICS
// Reset speed and acceleration to 0
inline void Car::ResetPhysics() {
  speed_ = default_speed();
  // centripeta_velocity_x_ = 0.0f;
  // centripeta_velocity_z_ = 0.0f;
}
// Reduce speed by amount
//   Corrects speed < 0 to 0
//   @param the amount to decrease the speed by
inline void Car::ReduceSpeed(float amount) {
  speed_ -= amount;
  if (speed() < 0) {
    speed_ = 0;
  }
}
// Reduce centripetal velocity by an amount
//   @param percentage amount
//   @warn should only be called when car is not in driving mode
inline void Car::ReduceCentriVelocity(float percentage) {
  percentage /= 100;
  centripeta_velocity_x_ *= percentage;
  centripeta_velocity_z_ *= percentage;
}

// MUTATORS
// Sets the displacement
inline void Car::set_displacement(glm::vec3 new_displacement) {
  displacement_ = new_displacement;
}

#endif

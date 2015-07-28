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

  private:

};


#endif

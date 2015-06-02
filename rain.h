/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * rain.h, Simple definition of Rain in openGL
 * 
 * This file is a simple implementation of rain in openGL
 * for more clarification read the comments describing each function
 * 
 */

#ifndef ASSIGN3_RAIN_H_
#define ASSIGN3_RAIN_H_

#include <vector>
#include "camera.h"
#include <random>
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

struct Particle {
  glm::vec3 pos;
  glm::vec3 speed;
  unsigned char r, g, b, a;
  float weight;
};

class Rain {
  public:
    // Constructor - Take the shader ID you want the rain to be rendered to
    // NOTE - This shader needs more specific set up than most and you should not try to use
    // anything but rain.vert and rain.frag to render this rain
    Rain(const GLuint &program_id);

    // Returns the VAO
    inline unsigned int rainvao() const;

    // Return the shader ID
    inline GLuint rainshader() const;

  private:
    
    // Maximum number of particles to be rendered
    const int MAX_PARTICLES_;

    Particle * particles_;

    Camera * camera_;

    // Generate the Mesh (based on functionality provided in lecture slides)
    void GenerateMesh();

    // Create the VAO
    unsigned int CreateVao();

    // VAO to store the rain 
    unsigned int rain_vao_;  

    GLuint particle_instance_buffer_;

    GLuint particle_position_buffer_;

    GLuint particle_colour_buffer_;

    // GLuint to store the shader ID
    GLuint rain_shader_; 

};

// =======================================================================// 
// The following functions are simple accessor functions                  //        
// =======================================================================//  

// Return the rainVAO
inline unsigned int rain::rainvao() const {
  return rain_vao_;
}

// Return the rain shader
inline GLuint rain::rainshader() const {
  return rain_shader_;
}

#endif

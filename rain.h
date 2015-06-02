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
#include <stdlib.h>
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
  float speed;
  glm::vec4 colour;
};

class Rain {
  public:
    // Constructor - Take the shader ID you want the rain to be rendered to
    // NOTE - This shader needs more specific set up than most and you should not try to use
    // anything but rain.vert and rain.frag to render this rain
    Rain(const GLuint &program_id);

    void Render(Camera * camera);

    // Returns the VAO
    inline unsigned int rainvao() const;

    // Return the shader ID
    inline GLuint rainshader() const;

  private:
    
    // Maximum number of particles to be rendered
    const int MAX_PARTICLES_;

    // Dynamic array of particles
    Particle * particles_;

    // Generate the Mesh (based on functionality provided in lecture slides)
    void GenerateMesh();

    // Create the VAO
    unsigned int CreateVao();

    // Initialises first set of particles
    void Init();

    // Updates the position of each particles
    void UpdatePosition();

    // VAO to store the rain 
    unsigned int rain_vao_;  

    // VBO for single particle instance
    GLuint particle_instance_buffer_;

    // VBO of positions for instances
    GLuint particle_position_buffer_;

    // Array that is sent to GPU
    GLfloat * particle_position_buffer_data_;

    // VBO of colours for instances
    GLuint particle_colour_buffer_;

    // Array that is sent to GPU
    GLfloat * particle_colour_buffer_data_;

    // GLuint to store the shader ID
    GLuint rain_shader_; 

};

// =======================================================================// 
// The following functions are simple accessor functions                  //        
// =======================================================================//  

// Return the rainVAO
inline unsigned int Rain::rainvao() const {
  return rain_vao_;
}

// Return the rain shader
inline GLuint Rain::rainshader() const {
  return rain_shader_;
}

#endif

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
#include "Skybox.h"
#include "camera.h"
#include "object.h"
#include "shaders/shaders.h"
#include <stdlib.h>
#include <ctime>
#include <random>
#include "glm/glm.hpp"
#include <GL/glew.h>
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
    Rain(const Shader &shader, const bool is_debug = false);

    // Destructor - Free memory allocated from constructor
    ~Rain();

    void Render(Camera &camera, Object * car, Skybox * skybox) const;
    
    // Updates the position of each particles
    void UpdatePosition();

    // Returns the VAO
    inline GLuint rainvao() const;

    // Return the shader ID
    inline Shader shader() const;

  private:
    
    // Maximum number of particles to be rendered
    const int MAX_PARTICLES_;

    // Dynamic array of particles
    Particle * particles_;

    // Generate the Mesh (based on functionality provided in lecture slides)
    void GenerateMesh();

    // Create the VAO
    GLuint CreateVao();

    // Initialises first set of particles
    void Init();

    // GLuint to store the shader uniforms and ID
    const Shader shader_;

    // Rain shader specific attributes
    const GLuint initialVerticesLoc_;
    const GLuint positionsLoc_;
    const GLuint colourLoc_;
    // Rain shader specific uniforms
    const GLuint camRightHandle_;
    const GLuint camUpHandle_;

    // VAO to store the rain 
    const GLuint rain_vao_;

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

    // Store the max positions we want the rain to be located in
    int maxx_;
    int maxy_;
    int maxz_;

};

// =======================================================================// 
// The following functions are simple accessor functions                  //        
// =======================================================================//  

// Return the rainVAO
inline GLuint Rain::rainvao() const {
  return rain_vao_;
}

// Return the rain shader
inline Shader Rain::shader() const {
  return shader_;
}

#endif

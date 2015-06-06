/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * Skybox.cc, Simple iplementation of a skybox in openGL
 * 
 * This file is a simple implementation of a skybox in openGL
 * for more clarification read the comments describing each function
 *
 * Inspiration drawn from http://learnopengl.com/#!Advanced-OpenGL/Cubemaps
 * 
 */

#ifndef ASSIGN3_Skybox_H_
#define ASSIGN3_Skybox_H_

#include <vector>
#include "camera.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Skybox {
  public:
    // Constructor - Take the shader ID you want the skybox to be rendered to
    // NOTE - This shader needs more specific set up than most and you should not try to use
    // anything but sky.vert and sky.frag to render this skybox
    Skybox(const GLuint program_id);

    // Returns the VAO
    inline unsigned int skyboxvao() const;

    // Returns the cubemap texture
    inline GLuint skyboxtex() const;

    // Return the shader ID
    inline GLuint skyshader() const;   

  private:
    // For more detailed comments so the implementation inside the implementation of this class (Skybox.cc)
    // Load in the textures
    GLuint loadCubeTex(std::vector<const GLchar*> faces);
    
    // Create the VAO
    unsigned int CreateVao();

    // VAO to store the skybox 
    unsigned int skybox_vao_;  

    // GLuint to store the cubemap texture
    GLuint skybox_tex_;

    // GLuint to store the shader ID
    GLuint skybox_shader_;    
};

// =======================================================================// 
// The following functions are simple accessor functions                  //        
// =======================================================================//  

// Return the skybox texture 
inline GLuint Skybox::skyboxtex() const {
  return skybox_tex_;
}

// Return the skyboxVAO
inline unsigned int Skybox::skyboxvao() const {
  return skybox_vao_;
}

// Return the skybox shader
inline GLuint Skybox::skyshader() const {
  return skybox_shader_;
}

#endif

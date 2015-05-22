/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * Skybox.cc, Simple iplementation of a skybox in openGL
 * 
 * This file is a simple implementation of a skybox in openGL
 * for more clarification read the comments describing each function
 * found in the actual implementation (Skybox.cc) 
 */

#ifndef ASSIGN3_Skybox_H_
#define ASSIGN3_Skybox_H_

#include <vector>
#include <string>
#include <cassert>


#include "model_data.h"
#include "model.h"
#include "camera.h"

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

class Skybox {
  public:
    // Constructor simply takes in the shader it wants to render the skybox to
    Skybox(const GLuint &program_id);

    // Returns the VAO
    inline unsigned int skyboxvao() const;

    // Returns the cubemap texture
    inline GLuint skyboxtex() const;

    // Return the shader ID
    inline GLuint skyshader() const;

    


  private:   

    GLuint loadCubeTex(std::vector<const GLchar*> faces);
    // Load in the textures
    unsigned int CreateVao();

    // VAO to store the skybox 
    unsigned int skybox_vao_;  

    // GLuint to store the cubemap texture
    GLuint skybox_tex_;

    // GLuint to store the shader ID
    GLuint skybox_shader_;

    
};


inline GLuint Skybox::skyboxtex() const {
  return skybox_tex_;
}


inline unsigned int Skybox::skyboxvao() const {
  return skybox_vao_;
}


inline GLuint Skybox::skyshader() const {
  return skybox_shader_;
}

#endif

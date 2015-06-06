/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * Water.h, Simple definition of Water in openGL
 * 
 * This file is a simple implementation of Water in openGL
 * for more clarification read the comments describing each function
 *
 * Inspiration drawn from http://jayconrod.com/posts/34/water-simulation-in-glsl
 * 
 */

#ifndef ASSIGN3_Water_H_
#define ASSIGN3_Water_H_

#include <vector>
#include <GL/glew.h>
#include <random>

#include "camera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Water {
  public:
    // Constructor - Take the shader ID you want the Water to be rendered to
    // NOTE - This shader needs more specific set up than most and you should not try to use
    // anything but water.vert and water.frag to render this water
    Water(const GLuint program_id);

    // Destructor
    ~Water();

    // Send the delat time
    void SendTime(float dt);

    // Returns the VAO
    inline unsigned int watervao() const;

    // Return the shader ID
    inline GLuint watershader() const;

    // Return the amount of indices   
    inline unsigned int water_index_count() const;

    // Return the amount of Vertices 
    inline unsigned int water_vertex_count() const;

    // Return the Water width
    inline unsigned int width() const;

    // Return the Water height
    inline unsigned int height() const;

  private:
    
    // Generate the Mesh (based on functionality provided in lecture slides)
    void GenerateMesh();

    // Create the VAO
    unsigned int CreateVao();

    // VAO to store the Water 
    unsigned int water_vao_;  

    // GLuint to store the shader ID
    GLuint water_shader_; 

    // Vector to hold indices
    unsigned int * indices_;

    // Total number of indicies
    unsigned int water_num_indices_;

    // Total number of vertices
    unsigned int water_num_vertices_;

    // Length of width
    unsigned int plane_width_;

    // Length of height
    unsigned int plane_height_;

    // Vector to hold Vertex data
    float * vertices_;
};

// =======================================================================// 
// The following functions are simple accessor functions                  //        
// =======================================================================//  

// Return the WaterVAO
inline unsigned int Water::watervao() const {
  return water_vao_;
}

// Return the Water index count
inline unsigned int Water::water_index_count() const {
  return water_num_indices_;
}

// Return the Water index count
inline unsigned int Water::water_vertex_count() const {
  return water_num_vertices_;
}

// Return the Water shader
inline GLuint Water::watershader() const {
  return water_shader_;
}

// Return the Water width
inline unsigned int Water::width() const {
  return plane_width_;
}

// Return the Water height
inline unsigned int Water::height() const {
  return plane_height_;
}

#endif

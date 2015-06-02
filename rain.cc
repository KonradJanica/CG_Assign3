/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * rain.cc, Simple implementation of a Rain in openGL
 * 
 * This file is a simple implementation of rain in openGL
 * for more clarification read the comments describing each function
 *
 * Inspiration drawn from http://jayconrod.com/posts/34/rain-simulation-in-glsl
 * 
 */

#include "rain.h"

Rain::Rain(const GLuint &program_id)
{
  MAX_PARTICLES_ = 500;
  particles_ = new Particle[MAX_PARTICLES_];
  rain_vao_ = CreateVao();

}

unsigned int rain::CreateVao()
{
  // 
  static const GLfloat vertices[] = { 
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
   -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
  };

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO containing a single instance of a particle
  glGenBuffers(1, &particle_instance_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_instance_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), )

  // VBO containing position of each particle instance
  glGenBuffers(1, &particle_position_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);

  // Initially set data to NULL
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // VBO containing colours of each particle instance
  glGenBuffers(1, &particle_colour_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);

  // Initially set data to NULL
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

  return vao;
}

// Create a mesh to be used for rain, based on code given in
// lecture 'proceduralWavy.cpp' but is essentially just basic 
// mesh generation anyway
// @warn - stores to class variables
void rain::GenerateMesh()
{
  // This Number will define how detailed the mesh will be
  unsigned int N = 3;

  // Define vertex data
  int plane_width = 200; // amount of columns
  int plane_height = 200; // amount of rows
  int total_vertices = (plane_width + 1) * (plane_height + 1);
  vertices_ = new float[ total_vertices * 3];
  rain_num_vertices_ = total_vertices;
  
  // define indices
  int numIndPerRow = plane_width * 2 + 2;
  int numIndDegensReq = (plane_height - 1) * 2;
  int numIndices = numIndPerRow * plane_height + numIndDegensReq;
  indices_ = new unsigned int[numIndices];
  rain_num_indices_ = numIndices;
  
  int width = plane_width+1;
  int height = plane_height+1;
  // set up mesh points
  int idxFlag = 0;
  for (int y=0;y < height;y++){
      for (int x=0;x < width;x++){
        // Fiddle with this to stretch (the y*0.1 part)
        //vertices_[idxFlag++] = (float)y+(y*0.1)/height;
          vertices_[idxFlag++] = (float)y + (y*0.1)/height;
          vertices_[idxFlag++] = 0.0f;
          vertices_[idxFlag++] = (float)x + (x*0.01)/width;
      }
  }

  // set up indices
  int i = 0;
  height--;
  for(int y = 0; y < height; y++)
  {
      int base = y * width;
      
      //indices[i++] = (uint16)base;
      for(int x = 0; x < width; x++)
      {
          indices_[i++] = (base + x);
          indices_[i++] = (base + width + x);
      }
      // add a degenerate triangle (except in a last row)
      if(y < height - 1)
      {
          indices_[i++] = ((y + 1) * width + (width - 1));
          indices_[i++] = ((y + 1) * width);
      }
  }
}
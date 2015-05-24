/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * Water.cc, Simple implementation of a Water in openGL
 * 
 * This file is a simple implementation of Water in openGL
 * for more clarification read the comments describing each function
 *
 * Inspiration drawn from http://jayconrod.com/posts/34/water-simulation-in-glsl
 * 
 */

#include "Water.h"

Water::Water(const GLuint &program_id)
{
  water_shader_ = program_id;
  glUseProgram(water_shader_);

  // Create the mesh for the 'water', stored into the 
  // class vectors indices_ and vertices_
  GenerateMesh();

  // Create the VAO based on the index and vertex data 
  water_vao_ = CreateVao();

  // SEND OUR WAVE DATA

  // Send number of waves
  int wavesHandle = glGetUniformLocation(water_shader_ , "numWaves");
  int heightHandle = glGetUniformLocation(water_shader_ , "waterHeight");

  if(wavesHandle == -1 || heightHandle == -1)
  {
    printf("Could not get uniforms for waves \n");
  }
  glUniform1f(wavesHandle,4);
  glUniform1f(heightHandle,-1.0);

  
  //glUniform1f(timeHandle, time+1); 
  std::random_device rd; 
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_real_distribution<> distr(-M_PI/3, M_PI/3);

  for (int i = 0; i < 4; ++i) {
    // Still need to actually SEND the uniforms

    char uniformName[256];
    memset(uniformName, 0, sizeof(uniformName));

    float amplitude = 0.5f / (i + 1);
    printf("amplitude[%d] = %f \n", i, amplitude);
    snprintf(uniformName, sizeof(uniformName), "amplitude[%d]", i);
    int amplitudeHandle = glGetUniformLocation(water_shader_, uniformName);
    if(amplitudeHandle == -1 )
    {
      printf("couldnt get amplitude[%d]\n", i);
    }
    glUniform1f(amplitudeHandle, amplitude);

    float wavelength = 8 * M_PI / (i + 1);
    printf("wavelength[%d] = %f \n", i, wavelength);
    snprintf(uniformName, sizeof(uniformName), "wavelength[%d]", i);
    int wavelengthHandle = glGetUniformLocation(water_shader_, uniformName);
    if(wavelengthHandle == -1)
    {
      printf("couldnt get wavelength[%d]\n", i);
    }
    glUniform1f(wavelengthHandle, wavelength/10.0);   

    float speed = 1.0f + 2*i;
    printf("speed[%d] = %f \n", i, speed);
    snprintf(uniformName, sizeof(uniformName), "speed[%d]", i);
    int speedHandle = glGetUniformLocation(water_shader_, uniformName);
    if(speedHandle == -1)
    {
      printf("couldnt get speed[%d]\n", i);
    }
    glUniform1f(speedHandle, speed);
  
    float angle = distr(eng);
    printf("angle[%d] = cos(%f), sin(%f) \n", i, cos(angle), sin(angle));
    snprintf(uniformName, sizeof(uniformName), "direction[%d]", i);
    int directionHandle = glGetUniformLocation(water_shader_, uniformName);
    if(directionHandle == -1)
    {
      printf("couldnt get direction[%d]\n", i);
    }
    glUniform2f(directionHandle, cos(angle), sin(angle));
    
  }


}

unsigned int Water::CreateVao()
{

  glUseProgram(water_shader_);


  unsigned int vaoHandle;
  glGenVertexArrays(1, &vaoHandle);
  glBindVertexArray(vaoHandle);

  int vertLoc = glGetAttribLocation(water_shader_, "a_vertex");

  if(vertLoc == -1)
  {
    printf("Couldnt get vertex location for water\n");
  }

  // Two buffer [vertex, index]
  unsigned int buffer[2];
  glGenBuffers(2, buffer);

  // Set vertex attribute
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*water_num_vertices_*3, vertices_, GL_STATIC_DRAW);
  glEnableVertexAttribArray(vertLoc);
  glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Index
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*water_num_indices_, indices_, GL_STATIC_DRAW);
    
    // Un-bind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  return vaoHandle;  
}

// Create a mesh to be used for water, based on code given in
// lecture 'proceduralWavy.cpp' but is essentially just basic 
// mesh generation anyway
// @warn - stores to class variables
void Water::GenerateMesh()
{
  // This Number will define how detailed the mesh will be
  unsigned int N = 3;

  // Define vertex data
  int plane_width = 100; // amount of columns
  int plane_height = 100; // amount of rows
  int total_vertices = (plane_width + 1) * (plane_height + 1);
  vertices_ = new float[ total_vertices * 3];
  water_num_vertices_ = total_vertices;
  
  // define indices
  int numIndPerRow = plane_width * 2 + 2;
  int numIndDegensReq = (plane_height - 1) * 2;
  int numIndices = numIndPerRow * plane_height + numIndDegensReq;
  indices_ = new unsigned int[numIndices];
  water_num_indices_ = numIndices;
  
  int width = plane_width+1;
  int height = plane_height+1;
  // set up mesh points
  int idxFlag = 0;
  for (int y=0;y < height;y++){
      for (int x=0;x < width;x++){
        // Fiddle with this to stretch (the y*0.1 part)
        //vertices_[idxFlag++] = (float)y+(y*0.1)/height;
          vertices_[idxFlag++] = (float)y/height;
          vertices_[idxFlag++] = 0.0f;
          vertices_[idxFlag++] = (float)x/width;
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
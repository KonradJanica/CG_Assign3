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

Water::Water(const GLuint program_id)
{
  water_shader_ = program_id;
  glUseProgram(water_shader_);

  // Create the mesh for the 'water', stored into the 
  // class vectors indices_ and vertices_
  GenerateMesh();

  // Create the VAO based on the index and vertex data 
  water_vao_ = CreateVao();

  // Handles for wave properties
  int mtlambientHandle = glGetUniformLocation(water_shader_, "mtl_ambient");
  int mtldiffuseHandle = glGetUniformLocation(water_shader_, "mtl_diffuse");
  int mtlspecularHandle = glGetUniformLocation(water_shader_, "mtl_specular");
  int shininessHandle = glGetUniformLocation(water_shader_, "shininess");

  if( mtlambientHandle == -1)
  {
    fprintf(stderr,"Could not find uniform: 'mtl_ambient' In: Water - Constructor\n This may cause unexpected behaviour in the program\n");
  }

  if( mtldiffuseHandle == -1)
  {
    fprintf(stderr,"Could not find uniform: 'mtl_diffuse' In: Water - Constructor\n This may cause unexpected behaviour in the program\n");
  }

  if( mtlspecularHandle == -1)
  {
    fprintf(stderr,"Could not find uniform: 'mtl_specular' In: Water - Constructor\n This may cause unexpected behaviour in the program\n");
  }

  // Create and send material properties for the water
  float mtlambient[3] = { 0.5, 0.5, 0.5 };          // ambient material
  float mtldiffuse[3] = { 0.5, 0.5, 0.5};           // diffuse material
  float mtlspecular[3] = { 1.0, 1.0, 1.0 };         // specular material MITCH - maybe change
  glUniform3fv(mtlambientHandle, 1, mtlambient);
  glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
  glUniform3fv(mtlspecularHandle, 1, mtlspecular);
  float mtlshininess = 32.0f;
  glUniform1fv(shininessHandle, 1, &mtlshininess);

  // Determine the number of waves we want, and send over to the shader
  int wavesHandle = glGetUniformLocation(water_shader_ , "numWaves");
  int numWaves = 15;
  if(wavesHandle == -1 )
  {
    fprintf(stderr,"Could not find uniform: 'numWaves' In: Water - Constructor\n This may cause unexpected behaviour in the program\n");
  }

  glUniform1f(wavesHandle,numWaves);  

  glUniform1f(wavesHandle,8);


  

   
  // Generate a random device used to randomly generate waves
  // N.B - It is not particularly important what the values for the distr are
  std::random_device rd; 
  std::mt19937 eng(rd()); 
  std::uniform_real_distribution<> distr(-1, 1);

  for (int i = 0; i < numWaves; ++i) {


    // Create a small buffer, as we have dynamic uniform names
    char uniformName[256];
    memset(uniformName, 0, sizeof(uniformName));

    float amplitude = 0.5f / (i + 1);
    //printf("amplitude[%d] = %f \n", i, amplitude);
    snprintf(uniformName, sizeof(uniformName), "amplitude[%d]", i);
    int amplitudeHandle = glGetUniformLocation(water_shader_, uniformName);
    if(amplitudeHandle == -1 )
    {
      fprintf(stderr,"Could not find uniform: amplitude[%d] In: Water - Constructor\n This may cause unexpected behaviour in the program\n", i);
    }

    glUniform1f(amplitudeHandle, amplitude * 5.0);


    float wavelength = 8 * M_PI / (i + 1);
    //printf("wavelength[%d] = %f \n", i, wavelength);
    snprintf(uniformName, sizeof(uniformName), "wavelength[%d]", i);
    int wavelengthHandle = glGetUniformLocation(water_shader_, uniformName);
    if(wavelengthHandle == -1)
    {
      fprintf(stderr,"Could not find uniform: wavelength[%d] In: Water - Constructor\n This may cause unexpected behaviour in the program\n", i);
    }

    glUniform1f(wavelengthHandle, wavelength * 2.0);   


    float speed = 1.0f + 2*i;
    //printf("speed[%d] = %f \n", i, speed);
    snprintf(uniformName, sizeof(uniformName), "speed[%d]", i);
    int speedHandle = glGetUniformLocation(water_shader_, uniformName);
    if(speedHandle == -1)
    {
      fprintf(stderr,"Could not find uniform: speed[%d] In: Water - Constructor\n This may cause unexpected behaviour in the program\n", i);
    }
    glUniform1f(speedHandle, speed);
  
    float angle = distr(eng);
    //printf("angle[%d] = cos(%f), sin(%f) \n", i, cos(angle), sin(angle));
    snprintf(uniformName, sizeof(uniformName), "direction[%d]", i);
    int directionHandle = glGetUniformLocation(water_shader_, uniformName);
    if(directionHandle == -1)
    {
      fprintf(stderr,"Could not find uniform: direction[%d] In: Water - Constructor\n This may cause unexpected behaviour in the program\n", i);
    }
    glUniform2f(directionHandle, cos(angle), sin(angle));
    
  }


}

// Deallocate the memory we allocated from the heap
Water::~Water()
{
  delete[] vertices_;
  delete[] indices_;
}

// Send the updated time variable so the waves actually move
void Water::SendTime(float dt)
{
  glUseProgram(water_shader_);

  int timeHandle = glGetUniformLocation(water_shader_, "time");
  if(timeHandle == -1)
  {
    fprintf(stderr,"Could not find uniform: 'time' In: Water - SendTime\n This may cause unexpected behaviour in the program\n");
  }
  glUniform1f(timeHandle, dt); 
}

unsigned int Water::CreateVao()
{
  // Switch to out water shader program and create a VAO 
  glUseProgram(water_shader_);
  unsigned int vaoHandle;
  glGenVertexArrays(1, &vaoHandle);
  glBindVertexArray(vaoHandle);

  int vertLoc = glGetAttribLocation(water_shader_, "a_vertex");

  if(vertLoc == -1)
  {
    fprintf(stderr,"Could not find uniform: 'a_vertex' In: Water - CreateVao\n This may cause unexpected behaviour in the program\n");
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
  // Define vertex data
  plane_width_ = 200;      // amount of columns
  plane_height_ = 200;     // amount of rows
  int total_vertices = (plane_width_ + 1) * (plane_height_ + 1);
  vertices_ = new float[ total_vertices * 3];
  water_num_vertices_ = total_vertices;
  
  // define indices
  int numIndPerRow = plane_width_ * 2 + 2;
  int numIndDegensReq = (plane_height_ - 1) * 2;
  int numIndices = numIndPerRow * plane_height_ + numIndDegensReq;
  indices_ = new unsigned int[numIndices];
  water_num_indices_ = numIndices;
  
  int width = plane_width_+1;
  int height = plane_height_+1;
  // set up mesh points
  int idxFlag = 0;
  for (int y=0;y < height;y++){
      for (int x=0;x < width;x++){
          // The addition of (y*0.1) stretches the mesh out
          vertices_[idxFlag++] = (float)y; //+ (y*0.1)/height;
          vertices_[idxFlag++] = 0.0f;
          vertices_[idxFlag++] = (float)x;// + (x*0.01)/width;
      }
  }

  // set up indices
  int i = 0;
  height--;
  for(int y = 0; y < height; y++)
  {
      int base = y * width;
      
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

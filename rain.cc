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

#define VALS_PER_VERT 3
#define VALS_PER_COLOUR 4
#define CUBE_NUM_TRIS 12      // number of triangles in a cube (2 per face)
#define CUBE_NUM_VERTICES 8     // number of vertices in a cube`

#include "rain.h"

Rain::Rain(const GLuint &program_id) : MAX_PARTICLES_(500), rain_shader_(program_id)
{
  particles_ = new Particle[MAX_PARTICLES_];
  particle_position_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 3];
  particle_colour_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 4];

  rain_vao_ = CreateVao();
  Init();
  //UpdatePosition();
}

unsigned int Rain::CreateVao()
{
  glUseProgram(rain_shader_);

  static const GLfloat vertices[] = { 
   -0.1f, -0.5f, 0.0f,
    0.1f, -0.5f, 0.0f,
   -0.1f,  0.5f, 0.0f,
    0.1f,  0.5f, 0.0f,
  };

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO containing a single instance of a particle
  glGenBuffers(1, &particle_instance_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_instance_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // VBO containing position of each particle instance
  glGenBuffers(1, &particle_position_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);

  // Initially set data to NULL
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // VBO containing colours of each particle instance
  glGenBuffers(1, &particle_colour_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);

  // Initially set data to NULL
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  return vao;

  // // Cube has 8 vertices at its corners
  // float cubeVertices[ CUBE_NUM_VERTICES*VALS_PER_VERT ] = {
  //    -1.0f, -1.0f, 1.0f ,
  //         1.0f, -1.0f, 1.0f ,
  //         1.0f,  1.0f, 1.0f ,
  //        -1.0f,  1.0f, 1.0f ,
  //        -1.0f, -1.0f, -1.0f ,
  //         1.0f, -1.0f, -1.0f ,
  //         1.0f,  1.0f, -1.0f ,
  //        -1.0f,  1.0f, -1.0f  
  // };

  // // Colours for each vertex; red, green, blue and alpha
  // float cubeColours[ CUBE_NUM_VERTICES*VALS_PER_COLOUR ] = {
  //     1.0f, 0.0f, 0.0f, 1.0f,
  //     0.0f, 1.0f, 0.0f, 1.0f,
  //     0.0f, 0.0f, 1.0f, 1.0f,
  //     1.0f, 1.0f, 0.0f, 1.0f,
  //     0.0f, 1.0f, 1.0f, 1.0f,
  //     1.0f, 0.0f, 1.0f, 1.0f,
  //           1.0f, 1.0f, 1.0f, 1.0f,
  //           0.0f, 0.0f, 0.0f, 1.0f
  // };

  //   // Each square face is made up of two triangles
  //   unsigned int indices[CUBE_NUM_TRIS * 3] = {
  //       0,1,2, 2,3,0,
  //       1,5,6, 6,2,1,
  //       5,4,7, 7,6,5,
  //       4,0,3, 3,7,4,
  //       3,2,6, 6,7,3,
  //       4,5,1, 1,0,4
  //   };
    

  // unsigned int vaoHandle;
  // glGenVertexArrays(1, &vaoHandle);
  // glBindVertexArray(vaoHandle);

  // int vertLoc = glGetAttribLocation(rain_shader_, "position");
  // int colourLoc = glGetAttribLocation(rain_shader_, "colour");

  // // Buffers to store position, colour and index data
  // unsigned int buffer[3];
  // glGenBuffers(3, buffer);

  // // Set vertex position
  // glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  // glBufferData(GL_ARRAY_BUFFER, 
  //                sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
  // glEnableVertexAttribArray(vertLoc);
  // glVertexAttribPointer(vertLoc, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

  // // Colour attributes
  // glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
  // glBufferData(GL_ARRAY_BUFFER, 
  //                sizeof(cubeColours), cubeColours, GL_STATIC_DRAW);
  // glEnableVertexAttribArray(colourLoc);
  // glVertexAttribPointer(colourLoc, VALS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, 0);

  // // Set element attributes. Notice the change to using GL_ELEMENT_ARRAY_BUFFER
  // // We don't attach this to a shader label, instead it controls how rendering is performed
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[2]);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
  //                sizeof(indices), indices, GL_STATIC_DRAW);   
  
  //   // Un-bind
  // glBindVertexArray(0);
  // glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  // return vaoHandle;
}

void Rain::Init()
{
  srand(time(NULL));
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    int rand_val = rand() % 10;
    particles_[i].pos = glm::vec3(((rand() % 100) / 100.0f) * i, ((rand() % 100) / 100.0f) * i, ((rand() % 100) / 100.0f) * i);
    particles_[i].speed = 0.01f;
    particles_[i].colour = glm::vec4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
    // particles_[i].colour.x = 0.0f;
    // particles_[i].colour.y = 0.0f;
    // particles_[i].colour.z = 0.3f;
    // particles_[i].colour.w = 1.0f;
  }
}

void Rain::UpdatePosition()
{
  // Updates particles and puts in data arrays
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    particles_[i].pos.y -= particles_[i].speed;
    if(particles_[i].pos.y < 0)
    {
      particles_[i].pos.y = 50.0f;
    }

    particle_position_buffer_data_[i*3 + 0] = particles_[i].pos.x;
    particle_position_buffer_data_[i*3 + 1] = particles_[i].pos.y;
    particle_position_buffer_data_[i*3 + 2] = particles_[i].pos.z;

    particle_colour_buffer_data_[i*4 + 0] = particles_[i].colour.x;
    particle_colour_buffer_data_[i*4 + 1] = particles_[i].colour.y;
    particle_colour_buffer_data_[i*4 + 2] = particles_[i].colour.z;
    particle_colour_buffer_data_[i*4 + 3] = particles_[i].colour.w;
  }
}

void Rain::Render(Camera * camera)
{
  glUseProgram(rain_shader_);

  glBindVertexArray(rain_vao_);

  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 3 * sizeof(GLfloat), particle_position_buffer_data_);

  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 4 * sizeof(GLfloat), particle_colour_buffer_data_);

  GLint mvHandle = glGetUniformLocation(rain_shader_, "modelview_matrix");
  if (mvHandle == -1) {
    printf("Rain could not find 'modelview_matrix' uniform\n");
  }

  GLuint initialVerticesHandle = glGetAttribLocation(rain_shader_, "initial_vertices");
  GLuint positionsHandle = glGetAttribLocation(rain_shader_, "displaced_vertices");
  GLuint colourHandle = glGetAttribLocation(rain_shader_, "colour");


  glm::mat4 view_matrix = camera->view_matrix();
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix));

  GLuint CameraRight_worldspace_ID  = glGetUniformLocation(rain_shader_, "CameraRight_worldspace");
  GLuint CameraUp_worldspace_ID  = glGetUniformLocation(rain_shader_, "CameraUp_worldspace");

  glUniform3f(CameraRight_worldspace_ID, view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]);
  glUniform3f(CameraUp_worldspace_ID   , view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]);

  glBindBuffer(GL_ARRAY_BUFFER, particle_instance_buffer_);
  glEnableVertexAttribArray(initialVerticesHandle);
  glVertexAttribPointer(initialVerticesHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glEnableVertexAttribArray(positionsHandle);
  glVertexAttribPointer(positionsHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);
  glEnableVertexAttribArray(colourHandle);
  glVertexAttribPointer(colourHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

  // Needed for instanced draws
  glVertexAttribDivisor(initialVerticesHandle, 0);    // Same every particle
  glVertexAttribDivisor(positionsHandle, 1);          // One per particle
  glVertexAttribDivisor(colourHandle, 1);             // One per particle

  // Equivalent to looping over all particles (with 4 vertices)
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES_);

  // glUseProgram(rain_shader_);

  // int modelviewHandle = glGetUniformLocation(rain_shader_, "modelview_matrix");
  // if (modelviewHandle == -1)
  //   exit(1);

  // // We reset the camera for this frame
  //   glm::mat4 cameraMatrix = camera->view_matrix();
    
  //   cameraMatrix = glm::scale(cameraMatrix, glm::vec3(5.0f));
    
  // // Set VAO to the square model and draw three in different positions
  // glBindVertexArray(rain_vao_);

  
  // glUniformMatrix4fv( modelviewHandle, 1, false, glm::value_ptr(cameraMatrix) );
  // glDrawElements(GL_TRIANGLES, CUBE_NUM_TRIS * 3, GL_UNSIGNED_INT, 0);  // New call
}
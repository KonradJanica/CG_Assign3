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

Rain::Rain(const GLuint &program_id) : MAX_PARTICLES_(500), rain_shader_(program_id)
{
  particles_ = new Particle[MAX_PARTICLES_];
  particle_position_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 3];
  particle_colours_buffer_data_ = new GLubyte[MAX_PARTICLES_ * 3];

  rain_vao_ = CreateVao();
  Init();
}

unsigned int Rain::CreateVao()
{
  glUseProgram(rain_shader_);

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
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

  return vao;
}

void Rain::Init()
{
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    particles_[i].pos = glm::vec3(i, 50.0f, i);
    particles_[i].speed = 0.01f;
    particles_[i].r = 0;
    particles_[i].g = 255;
    particles_[i].b = 0;
    particles_[i].a = 255;
  }
}

void Rain::UpdatePosition()
{
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    particles_[i].pos.y -= particles_[i].speed;

    particle_position_buffer_data_[i*3 + 0] = particles_[i].pos.x;
    particle_position_buffer_data_[i*3 + 1] = ;
    particle_position_buffer_data_[i*3 + 2] = ;

    particle_colour_buffer_data_[i*4 + 0] = ;
    particle_colour_buffer_data_[i*4 + 1] = ;
    particle_colour_buffer_data_[i*4 + 2] = ;
    particle_colour_buffer_data_[i*4 + 3] = ;
  }
}

void Rain::Render()
{
  glUseProgram(program_id);

  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 4 * sizeof(GLfloat), )

  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);


}
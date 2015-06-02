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

 #define DEG2RAD(x) ((x)*M_PI/180.0)

#define VALS_PER_VERT 3
#define VALS_PER_COLOUR 4
#define CUBE_NUM_TRIS 12      // number of triangles in a cube (2 per face)
#define CUBE_NUM_VERTICES 8     // number of vertices in a cube`

#include "rain.h"

Rain::Rain(const GLuint &program_id) : MAX_PARTICLES_(10000), rain_shader_(program_id)
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
   -0.01f, -0.05f, 0.0f,
    0.01f, -0.05f, 0.0f,
   -0.01f,  0.05f, 0.0f,
    0.01f,  0.05f, 0.0f,
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

 
}

// (((rand() % 100) / 100.0f) * i) / maxx
void Rain::Init()
{
  srand(time(NULL));
  int maxx = 50.0f;
  int maxy = 20.0f;
  int maxz = 50.0f;
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    int rand_val = rand() % 10;
    particles_[i].pos = glm::vec3(rand() % maxx, rand() % 30,rand() % maxz);
    particles_[i].speed = 0.1f;
    float red = (rand() % 49 + 24) / 255.0f;
    float green = (rand() % 77 + 17) / 255.0f;
    float blue = (rand() % 176 + 70) / 255.0f;
    particles_[i].colour = glm::vec4(red, green, blue, 1.0f);
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
    particles_[i].pos.x += particles_[i].speed;// * sin(DEG2RAD(60));
    if(particles_[i].pos.y < -5)
    {
      particles_[i].pos.y = 30.0f - (rand() % 5);
    }

    if (particles_[i].pos.x > 50.0f) 
    {
      particles_[i].pos.x = 0.0f;
    }



    particle_position_buffer_data_[i*3 + 0] = particles_[i].pos.x;
    particle_position_buffer_data_[i*3 + 1] = particles_[i].pos.y;
    particle_position_buffer_data_[i*3 + 2] = particles_[i].pos.z;

    particle_colour_buffer_data_[i*4 + 0] = particles_[i].colour.x;
    particle_colour_buffer_data_[i*4 + 1] = particles_[i].colour.y;
    particle_colour_buffer_data_[i*4 + 2] = particles_[i].colour.z;
    particle_colour_buffer_data_[i*4 + 3] = particles_[i].colour.w;

    // particle_colour_buffer_data_[i*4 + 0] = 0.0f;
    // particle_colour_buffer_data_[i*4 + 1] = 0.0f;
    // particle_colour_buffer_data_[i*4 + 2] = 1.0f;
    // particle_colour_buffer_data_[i*4 + 3] = particles_[i].colour.w;
  }
}

void Rain::Render(Camera * camera, Object * car, Skybox * skybox)
{
  glUseProgram(rain_shader_);

  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

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

  // Uncomment this block if we are going to try do reflective rain
  // int texHandle = glGetUniformLocation(rain_shader_, "skybox");
  // if (texHandle == -1) {
    
  //     fprintf(stderr, "Could not find uniform variables (RAIN - SKYCUBE)\n");
   
  // }

  // int camPosHandle = glGetUniformLocation(rain_shader_, "cameraPos");
  // if (camPosHandle == -1) {
  //   printf("Couldnt get the campos for raindrop reflections\n");
  // }
  // glUniformMatrix3fv(camPosHandle, 1, false, glm::value_ptr(camera->cam_pos()));

  GLuint initialVerticesHandle = glGetAttribLocation(rain_shader_, "initial_vertices");
  GLuint positionsHandle = glGetAttribLocation(rain_shader_, "displaced_vertices");
  GLuint colourHandle = glGetAttribLocation(rain_shader_, "colour");


  glm::mat4 view_matrix = camera->view_matrix();

  glm::mat4 object_translate = glm::translate(glm::mat4(1.0f), 
      glm::vec3(car->translation().x - 25.0f, 1.0f , car->translation().z - 15.0f));

  view_matrix = view_matrix * object_translate;

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

  glDisable(GL_BLEND);

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
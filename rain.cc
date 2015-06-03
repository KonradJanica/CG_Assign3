/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * rain.cc, Simple implementation of a Rain in openGL
 * 
 * This file is a simple implementation of rain in openGL
 * for more clarification read the comments describing each function
 *
 * Code tutorial - http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
 * 
 */


#include "rain.h"

Rain::Rain(const GLuint &program_id) : MAX_PARTICLES_(100000), rain_shader_(program_id)
{
  // Initialize the particle buffers
  particles_ = new Particle[MAX_PARTICLES_];
  particle_position_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 3];
  particle_colour_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 4];

  // Set the range in which the rain is generated
  maxx_ = 100.0f;
  maxy_ = 30.0f;
  maxz_ = 100.0f;

  // Run initialization
  rain_vao_ = CreateVao();
  Init();

}

Rain::~Rain()
{
  delete [] particles_;
  delete [] particle_position_buffer_data_;
  delete [] particle_colour_buffer_data_;

}

unsigned int Rain::CreateVao()
{
  glUseProgram(rain_shader_);

  // Initial declaration of the shape of the 'raindrop'
  // these values can be modified to change the shape of the rain
  static const GLfloat vertices[] = { 
   -0.01f, -0.05f, 0.0f,
    0.01f, -0.05f, 0.0f,
   -0.01f,  0.05f, 0.0f,
    0.01f,  0.05f, 0.0f,
  };

  // Create a VAO for the rain 
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

void Rain::Init()
{

  // Random generation engines to generate uniform floats between a range 
  std::random_device rd; 
  std::mt19937 eng(rd()); 
  // One for each axis
  std::uniform_real_distribution<> xgen(0,maxx_);
  std::uniform_real_distribution<> ygen(0,maxy_);
  std::uniform_real_distribution<> zgen(0,maxz_);
  // Initialize the position/speed/colour for all particles


  int maxx = 50.0f;
  int maxy = 20.0f;
  int maxz = 50.0f;

  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    // Randomly generate the positions of the particle
    particles_[i].pos = glm::vec3(xgen(eng), ygen(eng), zgen(eng));
    particles_[i].speed = 0.1f;

    // Slightly randomize colours between a certain range (Blue/Blue-Grey)
    float red = (rand() % 49 + 24) / 255.0f;
    float green = (rand() % 77 + 17) / 255.0f;
    float blue = (rand() % 176 + 70) / 255.0f;
    particles_[i].colour = glm::vec4(red, green, blue, 1.0f);
  }
}

void Rain::UpdatePosition()
{
  // Updates particles and puts in data arrays
  for (unsigned int i = 0; i < MAX_PARTICLES_; i++)
  {
    // Reduce y so the rain travels towards the ground
    particles_[i].pos.y -= particles_[i].speed;

    // Reduce x so rain appears to be sweeping across the scene
    particles_[i].pos.x += particles_[i].speed;

    // Boundary cases so that the rain particles 'reset' with a smal amount of randomization
    particles_[i].pos.x += particles_[i].speed;

    if(particles_[i].pos.y < -5)
    {
      particles_[i].pos.y = maxy_ - (rand() % 5);
    }

    if (particles_[i].pos.x > maxx_) 
    {
      particles_[i].pos.x = 0.0f;
    }

    // Store the particles data inside the buffer, to be put in to the VBO

    particle_position_buffer_data_[i*3 + 0] = particles_[i].pos.x;
    particle_position_buffer_data_[i*3 + 1] = particles_[i].pos.y;
    particle_position_buffer_data_[i*3 + 2] = particles_[i].pos.z;

    particle_colour_buffer_data_[i*4 + 0] = particles_[i].colour.x;
    particle_colour_buffer_data_[i*4 + 1] = particles_[i].colour.y;
    particle_colour_buffer_data_[i*4 + 2] = particles_[i].colour.z;
    particle_colour_buffer_data_[i*4 + 3] = particles_[i].colour.w;
  }
}

void Rain::Render(Camera * camera, Object * car, Skybox * skybox)
{
  glUseProgram(rain_shader_);

  // Enable blending so that rain is slightly transparent, transparency is set in the frag shader
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glBindVertexArray(rain_vao_);

  // Set the Buffer subdata to be the positions array that we filled in UpdatePosition()
  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 3 * sizeof(GLfloat), particle_position_buffer_data_);

  // Set the colour subdata to be the positions array that we filled in UpdatePosition()
  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 4 * sizeof(GLfloat), particle_colour_buffer_data_);

  // Get the handle for the Model * View Matrix
  GLint mvHandle = glGetUniformLocation(rain_shader_, "modelview_matrix");
  if (mvHandle == -1) {
    fprintf(stderr,"Could not find uniform: modelview_matrix In: Rain - Render\n");
  }

  GLuint initialVerticesHandle = glGetAttribLocation(rain_shader_, "initial_vertices");
  GLuint positionsHandle = glGetAttribLocation(rain_shader_, "displaced_vertices");
  GLuint colourHandle = glGetAttribLocation(rain_shader_, "colour");

  // Get the view matrix from the camera
  glm::mat4 view_matrix = camera->view_matrix();

  // Translate based on the car, so the rain follows the car
  glm::mat4 object_translate = glm::translate(glm::mat4(1.0f), 
      glm::vec3(car->translation().x, 1.0f , car->translation().z));

  // Translate the rain so its centre is roughly around the centre of the car
  glm::mat4 rain_translate = glm::translate(glm::mat4(1.0f), glm::vec3(-maxx_ / 2.0f, 0.0f, -maxz_ / 2.0f));

  // Apply the transformations
  view_matrix = view_matrix * object_translate * rain_translate;

  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix));

  // The cameras up vector is (0,1,0) transform it to world space by
  // by multiplying my camera->world (view) matrix inverse
  GLuint CamRight  = glGetUniformLocation(rain_shader_, "cam_right");
  GLuint CamUp  = glGetUniformLocation(rain_shader_, "cam_up");

  glUniform3f(CamRight, view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]);
  glUniform3f(CamUp   , view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]);

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


}


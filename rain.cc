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

Rain::Rain(const Shader &shader, const bool is_debug) :
  // Setup Constants
  MAX_PARTICLES_(100000),
  // Setup shader and uniforms
  shader_(shader),
  initialVerticesLoc_(glGetAttribLocation(shader_.Id, "initial_vertices")),
  positionsLoc_(      glGetAttribLocation(shader_.Id, "displaced_vertices")),
  colourLoc_(         glGetAttribLocation(shader_.Id, "colour")),
  camRightHandle_(glGetUniformLocation(shader_.Id, "cam_right")),
  camUpHandle_(   glGetUniformLocation(shader_.Id, "cam_up")),
  // Setup VAO
  rain_vao_(CreateVao())
{
  // DEBUGGING PRINTS to stderr (if error) and stdout (else)
  if (is_debug) {
    const char * shader_name = "Rain shader";
    // Check the attrib locations in debugging mode
    Shader::CheckAttrib(initialVerticesLoc_, "initialVerticesLoc_", shader_name);
    Shader::CheckAttrib(positionsLoc_, "positionsLoc_", shader_name);
    Shader::CheckAttrib(colourLoc_, "colourLoc_", shader_name);
    // Check the uniform locations in debugging mode
    Shader::CheckHandle(camRightHandle_, "camRightHandle_", shader_name);
    Shader::CheckHandle(camUpHandle_, "camUpHandle_", shader_name);
  }

  // Initialize the particle buffers
  particles_ = new Particle[MAX_PARTICLES_];
  particle_position_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 3];
  particle_colour_buffer_data_ = new GLfloat[MAX_PARTICLES_ * 4];

  // Set the range in which the rain is generated
  maxx_ = 100.0f;
  maxy_ = 30.0f;
  maxz_ = 100.0f;

  // Run initialization
  Init();

}

Rain::~Rain()
{
  delete [] particles_;
  delete [] particle_position_buffer_data_;
  delete [] particle_colour_buffer_data_;

}

GLuint Rain::CreateVao()
{
  glUseProgram(shader_.Id);

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

  // Unbind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

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


  // int maxx = 50.0f;
  // int maxy = 20.0f;
  // int maxz = 50.0f;

  for (int i = 0; i < MAX_PARTICLES_; i++)
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
  for (int i = 0; i < MAX_PARTICLES_; i++)
  {
    // Reduce y so the rain travels towards the ground
    particles_[i].pos.y -= particles_[i].speed;

    // Reduce x so rain appears to be sweeping across the scene
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

void Rain::Render(Camera &camera, Object * car, Skybox * skybox) const
{
  glUseProgram(shader_.Id);

  // Enable blending so that rain is slightly transparent, transparency is set in the frag shader
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  // Cull Appropriately
  glCullFace(GL_BACK);

  glBindVertexArray(rain_vao_);

  // Set the Buffer subdata to be the positions array that we filled in UpdatePosition()
  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 3 * sizeof(GLfloat), particle_position_buffer_data_);

  // Set the colour subdata to be the positions array that we filled in UpdatePosition()
  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_ * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES_ * 4 * sizeof(GLfloat), particle_colour_buffer_data_);

  // Translate based on the car, so the rain follows the car
  const glm::mat4 object_translate = glm::translate(glm::mat4(1.0f), 
      glm::vec3(car->translation().x, 1.0f , car->translation().z));

  // Translate the rain so its centre is roughly around the centre of the car
  const glm::mat4 rain_translate = glm::translate(glm::mat4(1.0f), glm::vec3(-maxx_ / 2.0f, 0.0f, -maxz_ / 2.0f));

  // Get the view and projection matrices from the camera
  const glm::mat4 &VIEW       = camera.view_matrix();
  const glm::mat4 &PROJECTION = camera.projection_matrix();
  // Calculate MVP
  const glm::mat4 MODELVIEW   = VIEW * object_translate * rain_translate;
  const glm::mat4 MVP         = PROJECTION * MODELVIEW;

  glUniformMatrix4fv(shader_.mvpHandle, 1, false, glm::value_ptr(MVP));

  // The cameras up vector is (0,1,0) transform it to world space by
  // by multiplying my camera->world (view) matrix inverse
  glUniform3f(camRightHandle_, VIEW[0][0], VIEW[1][0], VIEW[2][0]);
  glUniform3f(camUpHandle_   , VIEW[0][1], VIEW[1][1], VIEW[2][1]);

  glBindBuffer(GL_ARRAY_BUFFER, particle_instance_buffer_);
  glEnableVertexAttribArray(initialVerticesLoc_);
  glVertexAttribPointer(initialVerticesLoc_, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer_);
  glEnableVertexAttribArray(positionsLoc_);
  glVertexAttribPointer(positionsLoc_, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, particle_colour_buffer_);
  glEnableVertexAttribArray(colourLoc_);
  glVertexAttribPointer(colourLoc_, 4, GL_FLOAT, GL_FALSE, 0, 0);

  // Needed for instanced draws
  glVertexAttribDivisor(initialVerticesLoc_, 0);    // Same every particle
  glVertexAttribDivisor(positionsLoc_, 1);          // One per particle
  glVertexAttribDivisor(colourLoc_, 1);             // One per particle

  // Equivalent to looping over all particles (with 4 vertices)
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES_);

  // Unbind
  glBindVertexArray(0);

  glDisable(GL_BLEND);

}


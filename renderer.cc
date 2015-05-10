#include "renderer.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
Renderer::Renderer() : coord_vao_handle(0), is_debugging_(false), light_pos_(glm::vec4(0,0,0,0)), width_(640), height_(480) {
  camera_ = new Camera;
}

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
Renderer::Renderer(bool debug_flag) : coord_vao_handle(0), is_debugging_(debug_flag), light_pos_(glm::vec4(0,0,0,0)), width_(640), height_(480) {
  camera_ = new Camera;
}

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
Renderer::Renderer(const int &width, const int &height) : coord_vao_handle(0), is_debugging_(false), light_pos_(glm::vec4(0,0,0,0)), width_(width), height_(height) {
  camera_ = new Camera;
}

// Adds a model to the member vector
//   @param program_id, a shader program
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
void Renderer::AddModel(GLuint &program_id, const std::string &model_filename, const bool &is_car) {
  if (is_car) {
    car_ = new Model(program_id, model_filename, glm::vec3(-0.8,-0.3,10), glm::vec3(0,0,-1), glm::vec3(0,1,0), glm::vec3(0.3,0.3,0.3));
    car_->EnablePhysics(0.0001,0.00001,0);
  } else {
    Object * object = new Model(program_id, model_filename, glm::vec3(0,0,-10));
    objects_.push_back(object);
  }

  SetupLighting(program_id, glm::vec3(0,0,0), glm::vec3(0.7,0.7,1), glm::vec3(1,1,1));
}

// Renders all models in the vector member
//   Should be called in the render loop
//   Draws coordinate axis if their VAO exists
//   TODO just loop through entire object vector
void Renderer::Render() {
}

void Renderer::skyBox(GLuint &program_id)
{

  glUseProgram(program_id);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Setup Handles
  int mvHandle = glGetUniformLocation(program_id, "modelview_matrix");
  int normHandle = glGetUniformLocation(program_id, "normal_matrix");
  int texHandle = glGetUniformLocation(program_id, "texMap");
  if (mvHandle == -1 || normHandle==-1 || texHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  // Uniform variables defining material colours
  // These can be changed for each sphere, to compare effects
  int mtlambientHandle = glGetUniformLocation(program_id, "mtl_ambient");
  int mtldiffuseHandle = glGetUniformLocation(program_id, "mtl_diffuse");
  int mtlspecularHandle = glGetUniformLocation(program_id, "mtl_specular");
  int shininessHandle = glGetUniformLocation(program_id, "shininess");
  if ( mtlambientHandle == -1 ||
      mtldiffuseHandle == -1 ||
      mtlspecularHandle == -1 ||
      shininessHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find material uniform variables\n");
    }
  }

  const glm::mat4 &camera_matrix = camera_->camera_matrix();
  glm::mat4 camera_matrix2 = glm::translate(camera_matrix, glm::vec3(0.0f,10.0f,0.0f));
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 normMatrix;
  normMatrix = glm::mat3(camera_matrix2);
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(camera_matrix2) ); // Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  // Pass Surface Colours to Shader
  const glm::vec3 &vao_ambient = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_diffuse = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_specular = glm::vec3(0.5f,0.5f,0.5f);
  float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };  // ambient material
  float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };  // diffuse material
  float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };  // specular material
  glUniform3fv(mtlambientHandle, 1, mtlambient);
  glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
  glUniform3fv(mtlspecularHandle, 1, mtlspecular);
  float mtlshininess = 0.8f; 
  glUniform1fv(shininessHandle, 1, &mtlshininess);

  // Bind VAO and texture - Terrain
  
  for (unsigned int x = 0; x < 6; x++) {
    glBindVertexArray(terrain_->skyBoxVAOHandle); 
    glBindTexture(GL_TEXTURE_2D, terrain_->skytexture_[x]);

   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    
    //glDrawElements(GL_TRIANGLES, 6 * x, GL_UNSIGNED_INT, 0); // New call
    glDrawArrays(GL_TRIANGLE_STRIP, x*4, 4);
   
  }
}
// Renders only selected model
//   Should be called in the render loop
//   @param unsigned int index, index of model in the member vector
//   @warn does NOT draw axis coordinates
void Renderer::Render(unsigned int index) {
  assert(index < objects_.size() && "Trying to access index outside of objects_ vector");
  GLuint program_id = objects_.at(index)->program_id();
  glUseProgram(program_id);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1.0f);

  // Other Handles Setup
  int texHandle = glGetUniformLocation(program_id, "texMap");
  if (texHandle == -1) {
    if (is_debugging_) {
      fprintf(stderr, "Could not find uniform variables\n");
      exit(1);
    }
  }
  int mvHandle = glGetUniformLocation(program_id, "modelview_matrix");
  int normHandle = glGetUniformLocation(program_id, "normal_matrix");
  int lightposHandle = glGetUniformLocation(program_id, "light_pos");
  if (mvHandle == -1 || normHandle==-1 || lightposHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  // Update the light position
  float lightPos[4] = { light_pos_.x, light_pos_.y, light_pos_.z, light_pos_.w };	
  glUniform4fv(lightposHandle, 1, lightPos); 

  // Uniform variables defining material colours
  // These can be changed for each sphere, to compare effects
  int mtlambientHandle = glGetUniformLocation(program_id, "mtl_ambient");
  int mtldiffuseHandle = glGetUniformLocation(program_id, "mtl_diffuse");
  int mtlspecularHandle = glGetUniformLocation(program_id, "mtl_specular");
  int shininessHandle = glGetUniformLocation(program_id, "shininess");
  if ( mtlambientHandle == -1 ||
      mtldiffuseHandle == -1 ||
      mtlspecularHandle == -1 ||
      shininessHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find material uniform variables\n");
    }
  }

  const glm::mat4 &camera_matrix = camera_->camera_matrix();
  glm::mat3 normMatrix;
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  normMatrix = glm::mat3(camera_matrix);
  const glm::mat4 &transform_matrix = objects_.at(index)->transform();
  glm::mat4 position_matrix = camera_matrix * transform_matrix;
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(position_matrix) );	// Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  const std::vector<std::pair<unsigned int, GLuint> > &vao_texture_handle = objects_.at(index)->vao_texture_handle();
  for (unsigned int y = 0; y < vao_texture_handle.size(); ++y) {
    // Pass Surface Colours to Shader
    const glm::vec3 &vao_ambient = objects_.at(index)->ambient_surface_colours_at(y);
    const glm::vec3 &vao_diffuse = objects_.at(index)->diffuse_surface_colours_at(y);
    const glm::vec3 &vao_specular = objects_.at(index)->specular_surface_colours_at(y);
    float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
    float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
    float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
    glUniform3fv(mtlambientHandle, 1, mtlambient);
    glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
    glUniform3fv(mtlspecularHandle, 1, mtlspecular);
    float mtlshininess = objects_.at(index)->shininess_at(y);
    glUniform1fv(shininessHandle, 1, &mtlshininess);

    // Bind VAO
    glBindTexture( GL_TEXTURE_2D, vao_texture_handle.at(y).second );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glBindVertexArray( vao_texture_handle.at(y).first ); 
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    glDrawElements(GL_TRIANGLES, objects_.at(index)->points_per_shape_at(y), GL_UNSIGNED_INT, 0);	// New call
  }
}

// Draws only the moving car model
//   Should be called in the render loop
//   TODO later turn this into Draw(Object * object) and replace Render(index)
void Renderer::DrawCar() {
  assert(car_ != 0 && "Trying to access index outside of objects_ vector");
  GLuint program_id = car_->program_id();
  glUseProgram(program_id);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1.0f);

  // Other Handles Setup
  int texHandle = glGetUniformLocation(program_id, "texMap");
  if (texHandle == -1) {
    if (is_debugging_) {
      fprintf(stderr, "Could not find uniform variables\n");
      exit(1);
    }
  }
  int mvHandle = glGetUniformLocation(program_id, "modelview_matrix");
  int normHandle = glGetUniformLocation(program_id, "normal_matrix");
  int lightposHandle = glGetUniformLocation(program_id, "light_pos");
  if (mvHandle == -1 || normHandle==-1 || lightposHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  // Update the light position
  float lightPos[4] = { light_pos_.x, light_pos_.y, light_pos_.z, light_pos_.w };	
  glUniform4fv(lightposHandle, 1, lightPos); 

  // Uniform variables defining material colours
  // These can be changed for each sphere, to compare effects
  int mtlambientHandle = glGetUniformLocation(program_id, "mtl_ambient");
  int mtldiffuseHandle = glGetUniformLocation(program_id, "mtl_diffuse");
  int mtlspecularHandle = glGetUniformLocation(program_id, "mtl_specular");
  int shininessHandle = glGetUniformLocation(program_id, "shininess");
  if ( mtlambientHandle == -1 ||
      mtldiffuseHandle == -1 ||
      mtlspecularHandle == -1 ||
      shininessHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find material uniform variables\n");
    }
  }

  const glm::mat4 &camera_matrix = camera_->camera_matrix();
  glm::mat3 normMatrix;
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  normMatrix = glm::mat3(camera_matrix);
  const glm::mat4 &transform_matrix = car_->transform();
  glm::mat4 position_matrix = camera_matrix * transform_matrix;
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(position_matrix) );	// Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  const std::vector<std::pair<unsigned int, GLuint> > &vao_texture_handle = car_->vao_texture_handle();
  for (unsigned int y = 0; y < vao_texture_handle.size(); ++y) {
    // Pass Surface Colours to Shader
    const glm::vec3 &vao_ambient = car_->ambient_surface_colours_at(y);
    const glm::vec3 &vao_diffuse = car_->diffuse_surface_colours_at(y);
    const glm::vec3 &vao_specular = car_->specular_surface_colours_at(y);
    float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
    float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
    float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
    glUniform3fv(mtlambientHandle, 1, mtlambient);
    glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
    glUniform3fv(mtlspecularHandle, 1, mtlspecular);
    float mtlshininess = car_->shininess_at(y);
    glUniform1fv(shininessHandle, 1, &mtlshininess);

    // Bind VAO
    glBindTexture( GL_TEXTURE_2D, vao_texture_handle.at(y).second );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glBindVertexArray( vao_texture_handle.at(y).first ); 
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    glDrawElements(GL_TRIANGLES, car_->points_per_shape_at(y), GL_UNSIGNED_INT, 0);	// New call
  }

  // Update Physics
  car_->UpdateTransform();
}

// Render Coordinate Axis 
//   Used with Render(index)
//   @warn requires VAO from EnableAxis
//   @warn is already included in Render() all function
void Renderer::RenderAxis() {
  //Render Axis if VAO exists
  if (coord_vao_handle != 0) {
    glUseProgram(axis_program_id);
    glDisable(GL_DEPTH_TEST);

    // Modelview Setup
    int modelviewHandle1 = glGetUniformLocation(axis_program_id, "modelview_matrix");
    if (modelviewHandle1 == -1)
      exit(1);

    const glm::mat4 &camera_matrix = camera_->camera_matrix();
    glUniformMatrix4fv( modelviewHandle1, 1, false, glm::value_ptr(camera_matrix));

    // Set VAO to the square model and draw three in different positions
    glBindVertexArray(coord_vao_handle);

    glLineWidth(4.0f);
    glDrawElements(GL_LINES, 2*3, GL_UNSIGNED_INT, 0);	// New call. 2 vertices * 3 lines
    glEnable(GL_DEPTH_TEST);
  }
}

// Creates a VAO for x,y,z axis and binds it to a given shader for proper colouring
//   Render() checks if the VAO is created and then draws it
//   @param program_id, a shader program with different colours for x,y,z
//   @warn should only be called once, duplicate calls are irrelevant
void Renderer::EnableAxis(const GLuint &program_id) {
  axis_program_id = program_id;
  //  Build coordinate lines
  std::vector<unsigned int> coord_indices;
  coord_indices.push_back(0);
  coord_indices.push_back(1);
  coord_indices.push_back(2);
  coord_indices.push_back(3);
  coord_indices.push_back(4);
  coord_indices.push_back(5);
  std::vector<glm::vec3> coord_vertices;
  coord_vertices.push_back(glm::vec3(-100000.0,0.0,0.0));
  coord_vertices.push_back(glm::vec3(100000.0,0.0,0.0));
  coord_vertices.push_back(glm::vec3(0.0,-100000.0,0.0));
  coord_vertices.push_back(glm::vec3(0.0,100000.0,0.0));
  coord_vertices.push_back(glm::vec3(0.0,0.0,-100000.0));
  coord_vertices.push_back(glm::vec3(0.0,0.0,100000.0));

  //Create axis VAO
  glUseProgram(program_id);

  assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3); //Vec3 cannot be loaded to buffer this way

  glGenVertexArrays(1, &coord_vao_handle);
  glBindVertexArray(coord_vao_handle);

  int vertLoc = glGetAttribLocation(program_id, "a_vertex");

  // Buffers to store position, colour and index data
  unsigned int buffer[2];
  glGenBuffers(2, buffer);

  // Set vertex position
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, 
      sizeof(glm::vec3) * coord_vertices.size() , &coord_vertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(vertLoc);
  glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Set element attributes. Notice the change to using GL_ELEMENT_ARRAY_BUFFER
  // We don't attach this to a shader label, instead it controls how rendering is performed
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
      sizeof(unsigned int) * coord_indices.size(), &coord_indices[0], GL_STATIC_DRAW);   
  // Un-bind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// Setup Light Components into Uniform Variables for Shader
void Renderer::SetupLighting(const GLuint &program_id, const glm::vec3 &light_ambient, const glm::vec3 &light_diffuse, const glm::vec3 &light_specular, const GLint &light_toggle_in) {
  glUseProgram(program_id);
  // Uniform lighting variables
  int lightambientHandle = glGetUniformLocation(program_id, "light_ambient");
  int lightdiffuseHandle = glGetUniformLocation(program_id, "light_diffuse");
  int lightspecularHandle = glGetUniformLocation(program_id, "light_specular");
  int lightToggleHandle = glGetUniformLocation(program_id, "light_toggle");
  if ( lightambientHandle == -1 ||
      lightdiffuseHandle == -1 ||
      lightspecularHandle == -1 ||
      lightToggleHandle == -1) {
    printf("Error: can't find light uniform variables, Model.cc approx line 90\n");
  }

  float lightambient[3] = { light_ambient.x, light_ambient.y, light_ambient.z };	// ambient light components
  float lightdiffuse[3] = { light_diffuse.x, light_diffuse.y, light_diffuse.z };	// diffuse light components
  float lightspecular[3] = { light_specular.x, light_specular.y, light_specular.z };	// specular light components
  GLint light_toggle = light_toggle_in;

  glUniform3fv(lightambientHandle, 1, lightambient);
  glUniform3fv(lightdiffuseHandle, 1, lightdiffuse);
  glUniform3fv(lightspecularHandle, 1, lightspecular);    
  glUniform1i(lightToggleHandle, light_toggle);    
}

// Render the cliff terrain
//   @warn requires EnableTerrain()
void Renderer::RenderTerrain() {
  GLuint program_id = terrain_->terrain_program_id();
  glUseProgram(program_id);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Setup Handles
  int mvHandle = glGetUniformLocation(program_id, "modelview_matrix");
  int normHandle = glGetUniformLocation(program_id, "normal_matrix");
  int lightposHandle = glGetUniformLocation(program_id, "light_pos");
  int texHandle = glGetUniformLocation(program_id, "texMap");
  if (mvHandle == -1 || normHandle==-1 || lightposHandle == -1 || texHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  // Update the light position
  float lightPos[4] = { light_pos_.x, light_pos_.y, light_pos_.z, light_pos_.w };	
  glUniform4fv(lightposHandle, 1, lightPos); 

  // Uniform variables defining material colours
  // These can be changed for each sphere, to compare effects
  int mtlambientHandle = glGetUniformLocation(program_id, "mtl_ambient");
  int mtldiffuseHandle = glGetUniformLocation(program_id, "mtl_diffuse");
  int mtlspecularHandle = glGetUniformLocation(program_id, "mtl_specular");
  int shininessHandle = glGetUniformLocation(program_id, "shininess");
  if ( mtlambientHandle == -1 ||
      mtldiffuseHandle == -1 ||
      mtlspecularHandle == -1 ||
      shininessHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find material uniform variables\n");
    }
  }

  const glm::mat4 &camera_matrix = camera_->camera_matrix();
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 normMatrix;
  normMatrix = glm::mat3(camera_matrix);
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(camera_matrix) );	// Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  // Pass Surface Colours to Shader
  const glm::vec3 &vao_ambient = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_diffuse = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_specular = glm::vec3(0.5f,0.5f,0.5f);
  float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
  float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
  float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
  glUniform3fv(mtlambientHandle, 1, mtlambient);
  glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
  glUniform3fv(mtlspecularHandle, 1, mtlspecular);
  float mtlshininess = 0.8f; 
  glUniform1fv(shininessHandle, 1, &mtlshininess);

  // Bind VAO and texture - Terrain
  const std::vector<unsigned int> &terrain_vao_handle = terrain_->terrain_vao_handle();
  for (unsigned int x = 0; x < terrain_vao_handle.size(); ++x) {
    glBindVertexArray(terrain_vao_handle.at(x)); 
    glBindTexture(GL_TEXTURE_2D, terrain_->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    int amount = terrain_->indice_count();
    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }

  //////////////////////////
  // ROADS
  int amount = terrain_->road_indice_count();
  // glm::mat4 translated_road;
  // for (float x = -5; x < 5; ++x) {
  //   // We compute the normal matrix from the current modelview matrix
  //   // and give it to our program
  //   translated_road = glm::translate(camera_matrix, glm::vec3(0,0,x*2.0f));
  //   normMatrix = glm::mat3(mvHandle);
  //   glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(translated_road));	// Middle
  //   glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));
  //   // Bind VAO Road
  //   glBindVertexArray(terrain_->road_vao_handle()); 
  //   glBindTexture(GL_TEXTURE_2D, terrain_->road_texture());
  //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
  //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
  //   // We are using texture unit 0 (the default)
  //   glUniform1i(texHandle, 0);
  //
  //   glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  // }
  // for (float x = 5; x < 15; ++x) {
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  // translated_road = glm::rotate(camera_matrix, 45.0f, glm::vec3(0,1,0));
  // translated_road = glm::translate(translated_road, glm::vec3(-7.3f,0.3f,x*2.0f));
  const std::vector<unsigned int> &road_vao_handle = terrain_->road_vao_handle();
  for (unsigned int x = 0; x < road_vao_handle.size(); ++x) {
    // Bind VAO Road
    glBindVertexArray(road_vao_handle.at(x)); 
    glBindTexture(GL_TEXTURE_2D, terrain_->road_texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }
}

// Creates the Terrain object for RenderTerrain()
//   Creates Terrain VAOs
//   @warn terrain_ on heap, must be deleted after
void Renderer::EnableTerrain(const GLuint &program_id) {
  terrain_ = new Terrain(program_id);
}

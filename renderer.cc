#include "renderer.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely with debugging enabled
Renderer::Renderer(const bool &debug_flag) 
  : coord_vao_handle(0), is_debugging_(debug_flag) {
}

// Draws/Renders the passed in objects (with their models) to the scene
//   Should be called in the render loop
//   @param Object * object, an object to render
//   @param Camera * camera, to get the camera matrix and correctly position world
//   @warn object not const because it is possibly changed with UpdateTransform()
//   @warn this function is not responsible for NULL PTRs
void Renderer::Render(Object * object, const Camera * camera, const glm::vec4 &light_pos) const {
  GLuint program_id = object->program_id();
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

  const glm::mat4 &camera_matrix = camera->camera_matrix();
  glm::mat3 normMatrix;
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  normMatrix = glm::mat3(camera_matrix);
  const glm::mat4 &transform_matrix = object->transform();
  glm::mat4 position_matrix = camera_matrix * transform_matrix;
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(position_matrix) );	// Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  // Update the light position, transform from world coord to eye coord before sending
  if (light_pos.w == 0.0) {
    glm::vec4 lightDir = glm::vec4(normMatrix * glm::vec3(light_pos), 0.0f);
    glUniform4fv(lightposHandle, 1, glm::value_ptr(lightDir));
  }
  else {
    glm::vec4 lightPos = camera_matrix * light_pos;
    glUniform4fv(lightposHandle, 1, glm::value_ptr(lightPos));
  }

  const std::vector<std::pair<unsigned int, GLuint> > &vao_texture_handle = object->vao_texture_handle();
  for (unsigned int y = 0; y < vao_texture_handle.size(); ++y) {
    // Pass Surface Colours to Shader
    const glm::vec3 &vao_ambient = object->ambient_surface_colours_at(y);
    const glm::vec3 &vao_diffuse = object->diffuse_surface_colours_at(y);
    const glm::vec3 &vao_specular = object->specular_surface_colours_at(y);
    float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
    float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
    float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
    glUniform3fv(mtlambientHandle, 1, mtlambient);
    glUniform3fv(mtldiffuseHandle, 1, mtldiffuse);
    glUniform3fv(mtlspecularHandle, 1, mtlspecular);
    float mtlshininess = object->shininess_at(y);
    glUniform1fv(shininessHandle, 1, &mtlshininess);

    // Bind VAO
    glBindTexture( GL_TEXTURE_2D, vao_texture_handle.at(y).second );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glBindVertexArray( vao_texture_handle.at(y).first ); 
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    glDrawElements(GL_TRIANGLES, object->points_per_shape_at(y), GL_UNSIGNED_INT, 0);	// New call
  }

  // Update Physics
  if (object->IsPhysics()) {
    object->UpdateTransform();
  }
}

// Render Coordinate Axis 
//   Used with Render(index)
//   @warn requires VAO from EnableAxis
//   @warn is already included in Render() all function
void Renderer::RenderAxis(const Camera * camera) const {
  //Render Axis if VAO exists
  if (coord_vao_handle != 0) {
    glUseProgram(axis_program_id);
    glDisable(GL_DEPTH_TEST);

    // Modelview Setup
    int modelviewHandle1 = glGetUniformLocation(axis_program_id, "modelview_matrix");
    if (modelviewHandle1 == -1)
      exit(1);

    const glm::mat4 &camera_matrix = camera->camera_matrix();
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

// Draws/Renders the passed in terrain to the scene
//   @param Terrain * terrain, a terrain (cliffs/roads) to render
//   @param Camera * camera, to get the camera matrix and correctly position world
//   @param vec4 light_pos, The position of the Light for lighting
//   @warn Not responsible for NULL PTRs
void Renderer::Render(const Terrain * terrain, const Camera * camera, const glm::vec4 &light_pos) const {
  GLuint program_id = terrain->terrain_program_id();
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

  const glm::mat4 &camera_matrix = camera->camera_matrix();
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 normMatrix;
  normMatrix = glm::mat3(camera_matrix);
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(camera_matrix) ); // Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  // Update the light position, transform from world coord to eye coord before sending
  if (light_pos.w == 0.0) {
    glm::vec4 lightDir = glm::vec4(normMatrix * glm::vec3(light_pos), 0.0f);
    glUniform4fv(lightposHandle, 1, glm::value_ptr(lightDir));
  }
  else {
    glm::vec4 lightPos = camera_matrix * light_pos;
    glUniform4fv(lightposHandle, 1, glm::value_ptr(lightPos));
  }

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
  const std::vector<unsigned int> &terrain_vao_handle = terrain->terrain_vao_handle();
  for (unsigned int x = 0; x < terrain_vao_handle.size(); ++x) {
    glBindVertexArray(terrain_vao_handle.at(x)); 
    glBindTexture(GL_TEXTURE_2D, terrain->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    int amount = terrain->indice_count();
    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }

  //////////////////////////
  // ROADS
  int amount = terrain->road_indice_count();
  const std::vector<unsigned int> &road_vao_handle = terrain->road_vao_handle();
  for (unsigned int x = 0; x < road_vao_handle.size(); ++x) {
    // Bind VAO Road
    glBindVertexArray(road_vao_handle.at(x)); 
    glBindTexture(GL_TEXTURE_2D, terrain->road_texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    // We are using texture unit 0 (the default)
    glUniform1i(texHandle, 0);

    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }
}

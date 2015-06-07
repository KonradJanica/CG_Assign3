#include "renderer.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely with debugging enabled
Renderer::Renderer(const bool &debug_flag) 
  : coord_vao_handle(0), is_debugging_(debug_flag) {
}

//   Renders the passed in water to the scene
//   Should be called in the controller
//   @param Water * water, the skybox to render
//   @param Camera * camera, to get the camera matrix and correctly position world
//   @warn this function is not responsible for NULL PTRs
void Renderer::RenderWater(const Water * water, const Object* object, const Camera * camera, const Skybox * Sky) const
{
  glUseProgram(water->watershader());

  //printf("The Water shader ID is (inside of renderwater2) %d \n", water->watershader());
  int mvHandle = glGetUniformLocation(water->watershader(), "modelview_matrix");
  if (mvHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  int camPosHandle = glGetUniformLocation(water->watershader(), "cameraPos");
  if (camPosHandle == -1) {
    printf("Couldnt get the campos for water reflections\n");
  }
  glUniformMatrix3fv(camPosHandle, 1, false, glm::value_ptr(camera->cam_pos()));

  int texHandle = glGetUniformLocation(water->watershader(), "skybox");
  if (texHandle == -1) {
    
      fprintf(stderr, "Could not find uniform variables (WATER - SKYCUBE)\n");
   
  }

  int normHandle = glGetUniformLocation(water->watershader(), "normal_matrix");
  if (normHandle == -1) {
    if (is_debugging_) {
      fprintf(stderr, "Could not find uniform variables\n");
      exit(1);
    }
  }
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


  glm::mat4 view_matrix = camera->view_matrix();

  // Get only the needed components of the object's model matrix

  // Translation to put water where car is
  // Y component is fixed at -3.0f so that it does not follow car falling down
  glm::mat4 object_translate = glm::translate(glm::mat4(1.0f), 
      glm::vec3(object->translation().x, -3.0f, object->translation().z));

  // Translate to reposition the origin of the water
  float water_translate_x = water->width() / 2;
  float water_translate_z = water->height() / 2;
  glm::mat4 water_translate = glm::translate(glm::mat4(1.0f), glm::vec3(-water_translate_x, 0.0f, -water_translate_z));
  
  view_matrix = view_matrix * object_translate * water_translate;

  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix) );

  // Create and send normal matrix
  glm::mat3 normMatrix;
  normMatrix = glm::mat3(view_matrix);
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

  // Send the cubemap (for reflections)
  glBindTexture(GL_TEXTURE_CUBE_MAP, Sky->skyboxtex());
  glUniform1i(texHandle, 0);

  // Render the Skybox
  glBindVertexArray(water->watervao());

  // MITCH - TODO COnsider changing this to triangles, whichever gives most FPS
  glDrawElements(GL_TRIANGLE_STRIP, water->water_index_count(), GL_UNSIGNED_INT, 0 );
  glDisable(GL_BLEND);
}

//   Renders the passed in skybox to the scene
//   Should be called in the controller
//   @param Skybox * sky, the skybox to render
//   @param Camera * camera, to get the camera matrix and correctly position world
//   @warn this function is not responsible for NULL PTRs
void Renderer::RenderSkybox(const Skybox * Sky, const Camera * camera) const
{
  int mvHandle = glGetUniformLocation(Sky->skyshader(), "modelview_matrix");
  if (mvHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  int cubeHandle = glGetUniformLocation(Sky->skyshader(), "cubeMap");
  if (cubeHandle == -1) {
    if (is_debugging_) {
      fprintf(stderr, "Could not find uniform variables\n");
      exit(1);
    }
  }

  int texHandle = glGetUniformLocation(Sky->skyshader(), "skybox");
  if (texHandle == -1) {
    if (is_debugging_) {
      fprintf(stderr, "Could not find uniform variables\n");
      exit(1);
    }
  }

  // Draw skybox with depth testing off
  glDepthMask(GL_FALSE);
  glUseProgram(Sky->skyshader());
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glBindTexture(GL_TEXTURE_CUBE_MAP, Sky->skyboxtex());
  glUniform1i(texHandle, 0);
  
  // Create and send view matrix with translation stripped in order for skybox
  // to always be in thr right location
  glm::mat4 view_matrix = glm::mat4(glm::mat3(camera->view_matrix()));
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix) );

  // Render the Skybox
  glBindVertexArray(Sky->skyboxvao());
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // Re-enable depth testing
  glDepthMask(GL_TRUE);
}

// Draws/Renders the passed in objects (with their models) to the scene
//   Should be called in the render loop
//   @param Object * object, an object to render
//   @param Camera * camera, to get the camera matrix and correctly position world
//   @warn this function is not responsible for NULL PTRs
void Renderer::Render(const Object * object, const Camera * camera) const {
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
  if (mvHandle == -1 || normHandle==-1) {
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

  const glm::mat4 &view_matrix = camera->view_matrix();
  glm::mat3 normMatrix;
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  normMatrix = glm::mat3(view_matrix);
  const glm::mat4 &model_matrix = object->model_matrix();
  glm::mat4 modelview_matrix = view_matrix * model_matrix;
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(modelview_matrix) );	// Middle
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

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

    const glm::mat4 &view_matrix = camera->view_matrix();
    glUniformMatrix4fv( modelviewHandle1, 1, false, glm::value_ptr(view_matrix));

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
void Renderer::EnableAxis(const GLuint program_id) {
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
void Renderer::Render(const Terrain * terrain, const Camera * camera) const {
  GLuint program_id = terrain->terrain_program_id();
  glUseProgram(program_id);
  // glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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




  const glm::mat4 &view_matrix = camera->view_matrix();
  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 normMatrix;
  normMatrix = glm::mat3(view_matrix);
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix) ); // Middle
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

  int texHandle2 = glGetUniformLocation(program_id, "normMap");
  if(texHandle2 == -1)
  {
    printf("TERRAIN COULDNT FIND NORMAL MAPPINGS\n");
  }

  int bumpHandle = glGetUniformLocation(program_id, "isBumped");


  glBindTexture(GL_TEXTURE_2D, terrain->cliff_bump());

  glUniform1i(texHandle2, 1);
  glUniform1i(bumpHandle, 1);

  int texHandle3 = glGetUniformLocation(program_id, "mossMap");
  if(texHandle3 == -1)
  {
    printf("TERRAIN COULDNT FIND MOSS MAPPINGS\n");
  }
  glBindTexture(GL_TEXTURE_2D, terrain->road_bump());

  glUniform1i(texHandle3, 2);

  // Bind VAO and texture - Terrain
  const circular_vector<unsigned int> &terrain_vao_handle = terrain->terrain_vao_handle();
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
  
  glUniform1i(bumpHandle, 0);
  //////////////////////////
  // ROADS
  int amount = terrain->road_indice_count();
  const circular_vector<unsigned int> &road_vao_handle = terrain->road_vao_handle();
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

  glUniform1i(bumpHandle, 0);

  
}

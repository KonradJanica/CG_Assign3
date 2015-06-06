#include "renderer.h"

// Construct with a camera and verbose debugging mode
//   Creates a depth buffer (used for shadows)
//   Allows for Verbose Debugging Mode
//   @param camera, The camera object used to get view matrix
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely with debugging enabled
Renderer::Renderer(const bool debug_flag) :
  // Rendering objects
  shaders_(Shaders(debug_flag)),
  // Default vars
  coord_vao_handle_(debug_flag ? EnableAxis() : 0),
  // Debugging state
  is_debugging_(debug_flag) {

  // Setup depth buffer //TODO FIX THIS!
  unsigned int windowX = 1024, windowY = 1024;

  // generate namespace for the frame buffer 
  glGenFramebuffers(1, &frame_buffer_name_);
  //switch to our fbo so we can bind stuff to it
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_name_);

  // and depthbuffer
  glGenTextures(1, &depth_texture_);
  // create the depth texture and attach it to the frame buffer.
  glBindTexture(GL_TEXTURE_2D, depth_texture_);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, windowX, windowY, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  // glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, windowX, windowY, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // Set "renderedTexture" as our depth attachement
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture_, 0);

  // Instruct openGL that we won't bind a color texture with the currently binded FBO
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // Always check that our framebuffer is ok
  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      exit(-1);
  }

  // Unbind buffer
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//   Renders the passed in water to the scene
//   Should be called in the controller
//   @param Water * water, the skybox to render
//   @warn this function is not responsible for NULL PTRs
void Renderer::RenderWater(const Water * water, const Object* object, const Skybox * Sky, const Camera &camera) const
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
  glUniformMatrix3fv(camPosHandle, 1, false, glm::value_ptr(camera.cam_pos()));

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


  glm::mat4 view_matrix = camera.view_matrix();

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
      glActiveTexture(GL_TEXTURE0);
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
//   @warn this function is not responsible for NULL PTRs
void Renderer::RenderSkybox(const Skybox * Sky, const Camera &camera) const {
  int mvHandle = glGetUniformLocation(Sky->skyshader(), "modelview_matrix");
  if (mvHandle == -1) {
    if (is_debugging_) {
      assert(0 && "Error: can't find matrix uniforms\n");
    }
  }

  // TODO KONRAD -> MITCH not used?
  // int cubeHandle = glGetUniformLocation(Sky->skyshader(), "cubeMap");
  // if (cubeHandle == -1) {
  //   if (is_debugging_) {
  //     fprintf(stderr, "Could not find uniform variables\n");
  //     // exit(1);
  //   }
  // }

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

      glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, Sky->skyboxtex());
  glUniform1i(texHandle, 0);
  
  // Create and send view matrix with translation stripped in order for skybox
  // to always be in thr right location
  glm::mat4 view_matrix = glm::mat4(glm::mat3(camera.view_matrix()));
  glUniformMatrix4fv(mvHandle, 1, false, glm::value_ptr(view_matrix) );

  // Render the Skybox
  glBindVertexArray(Sky->skyboxvao());
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // Re-enable depth testing
  glDepthMask(GL_TRUE);
  // Unbind
  glBindVertexArray(0);
}

// Draws/Renders the passed in objects (with their models) to the scene
//   Should be called in the render loop
//   @param Object * object, an object to render
//   @warn this function is not responsible for NULL PTRs
void Renderer::Render(const Object * object, const Camera &camera) const {
  const Shader * shader = object->shader();
  glUseProgram(shader->Id);

  glCullFace(GL_BACK);
  // glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  const glm::mat4 PROJECTION = glm::perspective(75.0f, float(640 / 480), 0.1f, 100.0f);
  const glm::mat4 VIEW = camera.view_matrix();
  const glm::mat4 MODEL = object->model_matrix();
  const glm::mat4 MVP = PROJECTION * VIEW * MODEL;
  const glm::mat4 BIAS = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 0.5f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 0.5f, 0.0f,
                                   0.5f, 0.5f, 0.5f, 1.0f);

  // Compute the MVP matrix from the light's point of view
  const glm::mat4 D_PROJECTION = glm::ortho<float> (-100,100,-40,40,-100,100);
  const glm::vec2 texel_size = glm::vec2(1.0f/1024.0f, 1.0f/1024.0f);
  const glm::vec3 snapped_cam_pos = glm::vec3(
      floor(camera.cam_pos().x / texel_size.x) * texel_size.x,
      float(),
      floor(camera.cam_pos().z / texel_size.y) * texel_size.y);
  const glm::vec3 light_start = glm::vec3(snapped_cam_pos.x-35.0f,10.0f,snapped_cam_pos.z);
  const glm::vec3 light_end = glm::vec3(snapped_cam_pos.x+35.0f,-10.0f,snapped_cam_pos.z);
  const glm::mat4 D_VIEW = glm::lookAt(light_start, light_end, glm::vec3(0,1,0));
  const glm::mat4 D_MODEL = glm::mat4(1.0);
  const glm::mat4 DEPTH_MVP = D_PROJECTION * D_VIEW * D_MODEL;

  const glm::mat4 DEPTH_BIAS_MVP = BIAS * DEPTH_MVP;

  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 NORMAL;
  // NORMAL = glm::mat3(view_matrix);
  NORMAL = glm::inverse(glm::transpose(glm::mat3(VIEW)));
  glUniformMatrix4fv(shader->mvHandle, 1, false, glm::value_ptr(VIEW * MODEL));
  glUniformMatrix4fv(shader->mvpHandle, 1, false, glm::value_ptr(MVP));
  glUniformMatrix4fv(shader->depthBiasMvpHandle, 1, false, glm::value_ptr(DEPTH_BIAS_MVP));
  glUniformMatrix3fv(shader->normHandle, 1, false, glm::value_ptr(NORMAL));

  const std::vector<std::pair<unsigned int, GLuint> > &vao_texture_handle = object->vao_texture_handle();
  for (unsigned int y = 0; y < vao_texture_handle.size(); ++y) {
    // Pass Surface Colours to Shader
    const glm::vec3 &vao_ambient = object->ambient_surface_colours_at(y);
    const glm::vec3 &vao_diffuse = object->diffuse_surface_colours_at(y);
    const glm::vec3 &vao_specular = object->specular_surface_colours_at(y);
    float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
    float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
    float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
    float mtlshininess = object->shininess_at(y);
  glUniform3fv(shader->mtlAmbientHandle, 1, mtlambient);
  glUniform3fv(shader->mtlDiffuseHandle, 1, mtldiffuse);
  glUniform3fv(shader->mtlSpecularHandle, 1, mtlspecular);
  glUniform1fv(shader->shininessHandle, 1, &mtlshininess);

      // We are using texture unit 0 (the default)
      glActiveTexture(GL_TEXTURE0);
      glUniform1i(shader->texMapHandle, 0);

    // Bind VAO
    glBindTexture(GL_TEXTURE_2D, vao_texture_handle.at(y).second);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

      // // TODO move out of loop later
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, depth_texture_);
      glUniform1i(shader->shadowMapHandle, 1);

    // Populate Shader
    glBindVertexArray(vao_texture_handle.at(y).first); 
    // glBindAttribLocation(shader->Id, 0, "a_vertex");
    // glBindAttribLocation(shader->Id, 1, "a_normal");
    // glBindAttribLocation(shader->Id, 2, "a_texture");
    glDrawElements(GL_TRIANGLES, object->points_per_shape_at(y), GL_UNSIGNED_INT, 0);	// New call
  }
  // Un-bind
  glBindVertexArray(0);

}

// Render Coordinate Axis 
//   Only renders in debugging mode
//   @warn requires VAO from EnableAxis
void Renderer::RenderAxis(const Camera &camera) const {
  //Render Axis if Debugging mode
  if (is_debugging_) {
    printf("HERE\n");
    const Shader * shader = shaders_.AxisDebug;
    glUseProgram(shader->Id);
    glDisable(GL_DEPTH_TEST);

  const glm::mat4 PROJECTION = glm::perspective(75.0f, float(640 / 480), 0.1f, 100.0f);
    const glm::mat4 &view_matrix = camera.view_matrix();
    glUniformMatrix4fv(shader->mvHandle, 1, false, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(shader->projHandle, 1, false, glm::value_ptr(PROJECTION));

    glBindVertexArray(coord_vao_handle_);

    glLineWidth(4.0f);
    glDrawElements(GL_LINES, 2*3, GL_UNSIGNED_INT, 0);	// New call. 2 vertices * 3 lines
    glEnable(GL_DEPTH_TEST);
  }
}

// Creates a VAO for x,y,z axis and binds it to a given shader for proper colouring
//   @return a VAO to use for the Axis
//   @warn should only be called once, duplicate calls are irrelevant
GLuint Renderer::EnableAxis() const {
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

  GLuint coord_vao_handle;
  GLuint program_id = shaders_.AxisDebug->Id;
  glUseProgram(program_id);
int vertLoc = glGetAttribLocation(program_id, "a_vertex");

  //Create axis VAO
  glGenVertexArrays(1, &coord_vao_handle);
  glBindVertexArray(coord_vao_handle);

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

  return coord_vao_handle;
}

// Draws/Renders the passed in terrain to the scene
//   @param Terrain * terrain, a terrain (cliffs/roads) to render
//   @param vec4 light_pos, The position of the Light for lighting
//   @warn Not responsible for NULL PTRs
void Renderer::Render(const Terrain * terrain, const Camera &camera) const {
  const Shader * shader = terrain->shader();
  glUseProgram(shader->Id);
  glCullFace(GL_BACK);
  // glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  const glm::mat4 PROJECTION = glm::perspective(75.0f, float(640 / 480), 0.1f, 100.0f);
  const glm::mat4 VIEW = camera.view_matrix();
  const glm::mat4 MODEL = glm::mat4(1.0f);
  const glm::mat4 MVP = PROJECTION * VIEW * MODEL;
  const glm::mat4 BIAS = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 0.5f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 0.5f, 0.0f,
                                   0.5f, 0.5f, 0.5f, 1.0f);

  // Compute the MVP matrix from the light's point of view
  const glm::mat4 D_PROJECTION = glm::ortho<float> (-100,100,-40,40,-100,100);
  const glm::vec2 texel_size = glm::vec2(1.0f/1024.0f, 1.0f/1024.0f);
  const glm::vec3 snapped_cam_pos = glm::vec3(
      floor(camera.cam_pos().x / texel_size.x) * texel_size.x,
      float(),
      floor(camera.cam_pos().z / texel_size.y) * texel_size.y);
  const glm::vec3 light_start = glm::vec3(snapped_cam_pos.x-35.0f,10.0f,snapped_cam_pos.z);
  const glm::vec3 light_end = glm::vec3(snapped_cam_pos.x+35.0f,-10.0f,snapped_cam_pos.z);
  const glm::mat4 D_VIEW = glm::lookAt(light_start, light_end, glm::vec3(0,1,0));
  const glm::mat4 D_MODEL = glm::mat4(1.0);
  const glm::mat4 DEPTH_MVP = D_PROJECTION * D_VIEW * D_MODEL;

  const glm::mat4 DEPTH_BIAS_MVP = BIAS * DEPTH_MVP;

  // We compute the normal matrix from the current modelview matrix
  // and give it to our program
  glm::mat3 NORMAL;
  // NORMAL = glm::mat3(view_matrix);
  NORMAL = glm::inverse(glm::transpose(glm::mat3(VIEW)));
  glUniformMatrix4fv(shader->mvHandle, 1, false, glm::value_ptr(VIEW * MODEL));
  glUniformMatrix4fv(shader->mvpHandle, 1, false, glm::value_ptr(MVP));
  glUniformMatrix4fv(shader->depthBiasMvpHandle, 1, false, glm::value_ptr(DEPTH_BIAS_MVP));
  glUniformMatrix3fv(shader->normHandle, 1, false, glm::value_ptr(NORMAL));

  // Pass Surface Colours to Shader
  const glm::vec3 &vao_ambient = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_diffuse = glm::vec3(0.5f,0.5f,0.5f);
  const glm::vec3 &vao_specular = glm::vec3(0.5f,0.5f,0.5f);
  const float mtlambient[3] = { vao_ambient.x, vao_ambient.y, vao_ambient.z };	// ambient material
  const float mtldiffuse[3] = { vao_diffuse.x, vao_diffuse.y, vao_diffuse.z };	// diffuse material
  const float mtlspecular[3] = { vao_specular.x, vao_specular.y, vao_specular.z };	// specular material
  const float mtlshininess = 0.8f; 
  glUniform3fv(shader->mtlAmbientHandle, 1, mtlambient);
  glUniform3fv(shader->mtlDiffuseHandle, 1, mtldiffuse);
  glUniform3fv(shader->mtlSpecularHandle, 1, mtlspecular);
  glUniform1fv(shader->shininessHandle, 1, &mtlshininess);

  // Bind VAO and texture - Terrain
  const circular_vector<unsigned int> &terrain_vao_handle = terrain->terrain_vao_handle();
  for (unsigned int x = 0; x < terrain_vao_handle.size(); ++x) {
      // We are using texture unit 0 (the default)
      glActiveTexture(GL_TEXTURE0);
      glUniform1i(shader->texMapHandle, 0);

      glBindTexture(GL_TEXTURE_2D, terrain->texture());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

      // // TODO move out of loop later
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, depth_texture_);
      glUniform1i(shader->shadowMapHandle, 1);

    // Populate Shader
    glBindVertexArray(terrain_vao_handle.at(x));
    // glBindAttribLocation(shader->Id, 0, "a_vertex");
    // glBindAttribLocation(shader->Id, 1, "a_normal");
    // glBindAttribLocation(shader->Id, 2, "a_texture");
    int amount = terrain->indice_count();
    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }

  // ROADS
  glCullFace(GL_FRONT); //Road is rendered with reverse facing
  int amount = terrain->road_indice_count();
  const circular_vector<unsigned int> &road_vao_handle = terrain->road_vao_handle();
  for (unsigned int x = 0; x < road_vao_handle.size(); ++x) {
      // We are using texture unit 0 (the default)
      glActiveTexture(GL_TEXTURE0);
      glUniform1i(shader->texMapHandle, 0);
      // Bind VAO Road
      glBindTexture(GL_TEXTURE_2D, terrain->road_texture());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

      // // TODO move out of loop later
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, depth_texture_);
      glUniform1i(shader->shadowMapHandle, 1);

    // Populate shader
    glBindVertexArray(road_vao_handle.at(x));
    glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_INT, 0);	// New call
  }
  // Un-bind
  glBindVertexArray(0);
}

// Draws/Renders the passed in terrain to the scene
//   @param Terrain * terrain, a terrain (cliffs/roads) to render
//   @param vec4 light_pos, The position of the Light for lighting
//   TODO this is depth buffer
//   @warn Not responsible for NULL PTRs
void Renderer::RenderDepthBuffer(const Object * object, const Camera &camera) const {
  const Shader &shader = shaders_.DepthBuffer;
  glUseProgram(shader.Id);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Remove shadow acne
  glCullFace(GL_FRONT);

  // Compute the MVP matrix from the light's point of view
  const glm::mat4 PROJECTION = glm::ortho<float> (-100,100,-40,40,-100,100);
  const glm::vec2 texel_size = glm::vec2(1.0f/1024.0f, 1.0f/1024.0f);
  const glm::vec3 snapped_cam_pos = glm::vec3(
      floor(camera.cam_pos().x / texel_size.x) * texel_size.x,
      float(),
      floor(camera.cam_pos().z / texel_size.y) * texel_size.y);
  const glm::vec3 light_start = glm::vec3(snapped_cam_pos.x-35.0f,10.0f,snapped_cam_pos.z);
  const glm::vec3 light_end = glm::vec3(snapped_cam_pos.x+35.0f,-10.0f,snapped_cam_pos.z);
  const glm::mat4 VIEW = glm::lookAt(light_start, light_end, glm::vec3(0,1,0));
  const glm::mat4 MODEL = object->model_matrix();
  const glm::mat4 DEPTH_MVP = PROJECTION * VIEW * MODEL;

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(shader.depthMvpHandle, 1, GL_FALSE, glm::value_ptr(DEPTH_MVP));

  const std::vector<std::pair<unsigned int, GLuint> > &vao_texture_handle = object->vao_texture_handle();
  for (unsigned int y = 0; y < vao_texture_handle.size(); ++y) {
    // Pass Surface Colours to Shader
    glBindVertexArray( vao_texture_handle.at(y).first ); 
    glDrawElements(GL_TRIANGLES, object->points_per_shape_at(y), GL_UNSIGNED_INT, 0);	// New call
  }
  // Unbind
  glBindVertexArray(0);
}

// Draws/Renders the passed in terrain to the scene
//   @param Terrain * terrain, a terrain (cliffs/roads) to render
//   @param vec4 light_pos, The position of the Light for lighting
//   TODO this is depth buffer
//   @warn Not responsible for NULL PTRs
void Renderer::RenderDepthBuffer(const Terrain * terrain, const Camera &camera) const {
  const Shader &shader = shaders_.DepthBuffer;
  glUseProgram(shader.Id);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Remove shadow acne
  glCullFace(GL_FRONT);

  // Compute the MVP matrix from the light's point of view
  const glm::mat4 PROJECTION = glm::ortho<float> (-100,100,-40,40,-100,100);
  const glm::vec2 texel_size = glm::vec2(1.0f/1024.0f, 1.0f/1024.0f);
  const glm::vec3 snapped_cam_pos = glm::vec3(
      floor(camera.cam_pos().x / texel_size.x) * texel_size.x,
      float(),
      floor(camera.cam_pos().z / texel_size.y) * texel_size.y);
  const glm::vec3 light_start = glm::vec3(snapped_cam_pos.x-35.0f,10.0f,snapped_cam_pos.z);
  const glm::vec3 light_end = glm::vec3(snapped_cam_pos.x+35.0f,-10.0f,snapped_cam_pos.z);
  const glm::mat4 VIEW = glm::lookAt(light_start, light_end, glm::vec3(0,1,0));
  const glm::mat4 MODEL = glm::mat4(1.0);
  const glm::mat4 DEPTH_MVP = PROJECTION * VIEW * MODEL;

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(shader.depthMvpHandle, 1, GL_FALSE, glm::value_ptr(DEPTH_MVP));

  // Bind VAO and texture - Terrain
  const int amount_terrain = terrain->indice_count();
  const circular_vector<unsigned int> &terrain_vao_handle = terrain->terrain_vao_handle();
  for (unsigned int x = 0; x < terrain_vao_handle.size(); ++x) {
    glBindVertexArray(terrain_vao_handle.at(x));
    glDrawElements(GL_TRIANGLES, amount_terrain, GL_UNSIGNED_INT, 0);
  }

  // ROADS
  glCullFace(GL_BACK); //Road is rendererd reverse facing
  const int amount_road = terrain->road_indice_count();
  const circular_vector<unsigned int> &road_vao_handle = terrain->road_vao_handle();
  for (unsigned int x = 0; x < road_vao_handle.size(); ++x) {
    glBindVertexArray(road_vao_handle.at(x));
    glDrawElements(GL_TRIANGLES, amount_road, GL_UNSIGNED_INT, 0);
  }
  // Unbind
  glBindVertexArray(0);
}

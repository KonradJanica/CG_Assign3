#include "model.h"

Model::Model(GLuint *program_id, const std::string &model_filename, const bool &is_wireframe) : program_id_( program_id ), amount_points_(0), is_wireframe_(is_wireframe) {
  model_data_ = new ModelData( model_filename );
  subdir_ = model_filename.substr(0, model_filename.find_last_of('/') + 1);
  max_x_ = model_data_->GetMax( ModelData::kX );
  max_y_ = model_data_->GetMax( ModelData::kY );
  max_z_ = model_data_->GetMax( ModelData::kZ );
  min_x_ = model_data_->GetMin( ModelData::kX );
  min_y_ = model_data_->GetMin( ModelData::kY );
  min_z_ = model_data_->GetMin( ModelData::kZ );
  max_ = max_x_;
  if (max_y_ > max_)
    max_ = max_y_;
  if (max_z_ > max_)
    max_ = max_z_;
  min_ = min_x_;
  if (min_y_ < min_)
    min_ = min_y_;
  if (min_z_ < min_)
    min_ = min_z_;

  if (is_wireframe_) {
    ConstructWireframeModel();
  } else {
    ConstructShadedModel();
  }

  delete model_data_;
}

void Model::ConstructWireframeModel() {
  // Add VAO for every shape
  RawModelData::Shape* next_shape = model_data_->shape_at(0);
  assert(next_shape != NULL && "There are no shapes");  
  unsigned int shape_index = 0;
  std::pair<unsigned int, GLuint> next_handle;
  while (next_shape != NULL) {
    // Pair VAO and Texture into vao_texture_handle_
    next_handle.first = CreateVaoWireframe(next_shape);
    next_handle.second = 0;
    vao_texture_handle_.push_back(next_handle);
    shape_index++;
    next_shape = model_data_->shape_at(shape_index);
  }
}

void Model::ConstructShadedModel() {
  // Parse Materials to material_container
  std::vector<RawModelData::Material*> material_container;
  RawModelData::Material* next_material = model_data_->material_at(0);
  assert(next_material != NULL && "There are no materials");
  unsigned int material_index = 0;
  while (next_material != NULL) {
    material_container.push_back(next_material);

    material_index++;
    next_material = model_data_->material_at(material_index);
  }

  // Add VAO for every shape
  RawModelData::Shape* next_shape = model_data_->shape_at(0);
  assert(next_shape != NULL && "There are no shapes");  
  unsigned int shape_index = 0;
  std::pair<unsigned int, GLuint> next_handle;
  while (next_shape != NULL) {
    RawModelData::Material *working_material = material_container.at(next_shape->material_id);
    // Pair VAO and Texture into vao_texture_handle_
    next_handle.first = CreateVao( next_shape );
    next_handle.second = CreateTextures(working_material);
    vao_texture_handle_.push_back( next_handle );
    // Create Surface Colours Corresponding to vao_texture_handle_ indexes
    ambient_surface_colours_.push_back(working_material->ambient);
    diffuse_surface_colours_.push_back(working_material->diffuse);
    specular_surface_colours_.push_back(working_material->specular);
    shininess_.push_back(working_material->shininess);

    shape_index++;
    next_shape = model_data_->shape_at(shape_index);
  }
}

// Creates a new vertex array object and loads in data into a vertex attribute buffer
//   Creates only wireframe VAOs
//   Now we are associating two attributes with our VAO
//   @return vao_handle, the vao handle
unsigned int Model::CreateVaoWireframe(const RawModelData::Shape *shape) {
  glUseProgram( *program_id_ );
  const std::vector<unsigned int>& indices = shape->indices;
  const std::vector<glm::vec3>& vertices = shape->vertices;
  assert( indices.size() % 3 == 0 );
  amount_points_ += indices.size();
  points_per_shape_.push_back(indices.size());

  assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3); //Vec3 cannot be loaded to buffer this way

  unsigned int vao_handle;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);

  int vertLoc = glGetAttribLocation(*program_id_, "a_vertex");

  // Buffers to store position, colour and index data
  unsigned int buffer[2];
  glGenBuffers(2, buffer);

  // Set vertex position
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size() , &vertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(vertLoc);
  glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  // Set element attributes. Notice the change to using GL_ELEMENT_ARRAY_BUFFER
  // We don't attach this to a shader label, instead it controls how rendering is performed
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
      sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);   
  // Un-bind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vao_handle;
}

// Creates a new vertex array object and loads in data into a vertex attribute buffer
//   Now we are associating two attributes with our VAO
//   @return vao_handle, the vao handle
unsigned int Model::CreateVao ( const RawModelData::Shape *shape ) {
  glUseProgram( *program_id_ );
  const std::vector<unsigned int>& indices = shape->indices;
  const std::vector<glm::vec3>& vertices = shape->vertices;
  const std::vector<glm::vec3>& normals = shape->normals;
  std::vector<glm::vec2> texture_coordinates_uv = shape->texture_coordinates_uv;
  if ( texture_coordinates_uv.size() == 0 ) {
    std::vector<glm::vec2> dummy_uv;
    dummy_uv.push_back( glm::vec2(0,0) );
    dummy_uv.push_back( glm::vec2(0,1) );
    dummy_uv.push_back( glm::vec2(1,1) );
    dummy_uv.push_back( glm::vec2(1,0) );
    texture_coordinates_uv = dummy_uv;
  }

  assert( indices.size() % 3 == 0 );
  amount_points_ += indices.size();
  points_per_shape_.push_back(indices.size());

  assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3); //Vec3 cannot be loaded to buffer this way

  unsigned int vao_handle;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);

  int vertLoc = glGetAttribLocation(*program_id_, "a_vertex");
  int textureLoc = glGetAttribLocation(*program_id_, "a_texture");
	int normLoc = glGetAttribLocation(*program_id_, "a_normal");

  // Buffers to store position, colour and index data
  unsigned int buffer[4];
  glGenBuffers(4, buffer);

  // Set vertex position
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size() , &vertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(vertLoc);
  glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Normal attributes
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(normLoc);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  // Texture attributes
  glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texture_coordinates_uv.size(), &texture_coordinates_uv[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(textureLoc);
  glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
  // Set element attributes. Notice the change to using GL_ELEMENT_ARRAY_BUFFER
  // We don't attach this to a shader label, instead it controls how rendering is performed
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
      sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);   
  // Un-bind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vao_handle;
}

// Creates a texture pointer from file
//   @return new_texture, a GLuint texture pointer
GLuint Model::CreateTextures( const RawModelData::Material *material ) {
  const std::string& ambient_texture = material->ambient_texture;
  const std::string& diffuse_texture = material->diffuse_texture;
  const std::string& specular_texture = material->specular_texture;
  const std::string& normal_texture = material->normal_texture;

  // A shader program has many texture units, slots in which a texture can be bound, available to
  // it and this function defines which unit we are working with currently
  // We will only use unit 0 until later in the course. This is the default.
  glActiveTexture(GL_TEXTURE0);

  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  GLuint new_texture;

  //Check 0 or 1 texture in material
  unsigned int texture_count = 0;
  if (ambient_texture.size() > 0)
    texture_count++;
  if (diffuse_texture.size() > 0)
    texture_count++;
  if (specular_texture.size() > 0)
    texture_count++;
  if (normal_texture.size() > 0)
    texture_count++;
  //assert(texture_count <= 1 && "More than 1 texture found in material");
  glGenTextures( 1, &new_texture );

  // load an image from file as texture 1
  int x, y, n;
  unsigned char *data;
  if (texture_count == 0) {
    data = stbi_load(
        "models/white_for_null.png", /*char* filepath */
        // "crate.jpg",
        &x, /*The address to store the width of the image*/
        &y, /*The address to store the height of the image*/
        &n  /*Number of channels in the image*/,
        0   /*Force number of channels if > 0*/
        );
  } else {
    std::string filename = subdir_;
    if (ambient_texture.size() > 0) {
      filename += ambient_texture;
    } else if (specular_texture.size() > 0) {
      filename += specular_texture;
    } else if (normal_texture.size() > 0) {
      filename += normal_texture;
    } else if (diffuse_texture.size() > 0) {
      filename += diffuse_texture;
    }
    data = stbi_load(
        filename.c_str(), /*char* filepath */
        // "crate.jpg",
        &x, /*The address to store the width of the image*/
        &y, /*The address to store the height of the image*/
        &n  /*Number of channels in the image*/,
        0   /*Force number of channels if > 0*/
        );
  }

  glBindTexture( GL_TEXTURE_2D, new_texture );
  if (n == 3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  else {
    fprintf(stderr, "Image pixels are not RGB. You will need to change the glTexImage2D command.");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  stbi_image_free(data);

  glGenerateMipmap(GL_TEXTURE_2D); 

  return new_texture;
}

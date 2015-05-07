#include "terrain.h"

Terrain::Terrain(const GLuint &program_id, const int &width, const int &height) : terrain_program_id_(program_id), width_(width), height_(height) {
  // New Seed
  srand(time(NULL));
  // Setup Vars
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texture_coordinates_uv;
  std::vector<int> indices;
  std::vector<float> heights; // Uses this vector to build heights, smooths connections with this too
  heights.resize(width_ * height_, 0.15f);
  next_tile_start_ = glm::vec2(-10,-10);

  // Terrain
  // 1st buffer
  texture_ = LoadTexture("textures/rock01.jpg");
  GenerateTerrain(vertices, normals, texture_coordinates_uv, indices, heights);
  terrain_vao_handle_.push_back(CreateVao(terrain_program_id_, vertices, normals, texture_coordinates_uv, indices));
  // 3rd buffer
  GenerateTerrainTurn(vertices, normals, texture_coordinates_uv, indices, heights);
  terrain_vao_handle_.push_back(CreateVao(terrain_program_id_, vertices, normals, texture_coordinates_uv, indices));

  // Road
  road_texture_ = LoadTexture("textures/road.jpg");
  GenerateTerrain(vertices, normals, texture_coordinates_uv, indices, heights, true);
  road_vao_handle_ = CreateVao(terrain_program_id_, vertices, normals, texture_coordinates_uv, indices);
}

void Terrain::GenerateTerrain(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals,
    std::vector<glm::vec2> &texture_coordinates_uv, std::vector<int> &indices, std::vector<float> &float_heights_y, const bool &is_road) {
  // Setup Vars
  // float MIN_POSITION = -10.0f;
  float MIN_POSITION = 0.0f;
  float POSITION_RANGE = 20.0f;
  int x_length = width_;
  int z_length = height_;
  if (is_road) {
    x_length = width_/8;
    z_length = height_/8;
  // MIN_POSITION = -0.35f; // Good for 0 height
  MIN_POSITION = 0.0f;
  POSITION_RANGE = 2.0f;
  }

  // Store the connecting row to smooth
  std::vector<float> temp_last_row_heights;
  for (unsigned int x = float_heights_y.size()-1; x >= float_heights_y.size()-1-x_length; --x) {
    temp_last_row_heights.push_back(float_heights_y.at(x));
  }

  float_heights_y.clear();
  // float_heights_y.resize(x_length*z_length, -0.5f);
  float_heights_y.resize(x_length*z_length, 0.15f);
  if (!is_road) {
    for (unsigned int y = 0; y < z_length; ++y) {
      for (unsigned int x = 0; x < x_length; ++x) {
        // Normalize x between -1 and 1
        float norm_x = (float)x / (x_length-1);
        norm_x *= 2.0;
        norm_x -= 1.0;

        // Height modelled using X^3
        float_heights_y.at(x+y*x_length) = 20*(norm_x*norm_x*norm_x);
      }
    }

    // Even out little section right of road
    for (unsigned int x = 0; x < z_length; ++x) {
      float_heights_y.at(x_length/2-1 + x*x_length) = 0.168f;
      float_heights_y.at(x_length/2 + x*x_length) = 0.14f;
    }

    // Randomize Top Terrain
    int center_left_x = x_length/2 - x_length/4;
    int center_z = z_length - z_length/2;
    int x_position = center_left_x, z_position = center_z;
    for (unsigned int i = 0; i < 10000; ++i) {
      int v = rand() % 4 + 1;
      switch(v) {
        case 1: x_position++;
                break;
        case 2: x_position--;
                break;
        case 3: z_position++;
                break;
        case 4: z_position--;
                break;
      }
      if (x_position < 0) {
        x_position = x_length/2-2;
        // x_position = rand() % x_length/2;
        continue;
      } else if (x_position > x_length/2-2) {
        // x_position = x_length/2;
        // x_position = rand() % x_length/2;
        x_position = 0;
        continue;
      }
      if (z_position < 0) {
        // z_position = 0;
        // z_position = rand() % z_length;
        z_position = z_length-1;
        continue;
      } else if (z_position > z_length-1) {
        // z_position = z_length-1;
        // z_position = rand() % z_length;
        z_position = 0;
        continue;
      }
      float_heights_y.at(x_position + z_position*x_length) -= 0.100f;
    }

    // Randomize Bottom Terrain
    // x_position = x_length/2, z_position = z_length-1;
    int center_right_x = x_length - x_length/2;
    x_position = center_right_x, z_position = center_z;
    for (unsigned int i = 0; i < 10000; ++i) {
      int v = rand() % 4 + 1;
      switch(v) {
        case 1: x_position++;
                break;
        case 2: x_position--;
                break;
        case 3: z_position++;
                break;
        case 4: z_position--;
                break;
      }
      if (x_position < x_length/2+4) {
        // x_position = x_length/2;
        // x_position = rand() % x_length/2 + x_length/2;
        x_position = x_length-1;
        continue;
      } else if (x_position > x_length-1) {
        // x_position = x_length-1;
        // x_position = rand() % x_length/2 + x_length/2;
        x_position = x_length/2+4;
        continue;
      }
      if (z_position < 0) {
        // z_position = 0;
        // z_position = rand() % z_length;
        z_position = z_length-1;
        continue;
      } else if (z_position > z_length-1) {
        // z_position = z_length-1;
        // z_position = rand() % z_length;
        z_position = 0;
        continue;
      }
      float_heights_y.at(x_position + z_position*x_length) += 0.100f;
    }

    // TODO someone try fighting with this if you dare...
    //   Something goes wrong with the normals at the connection
    // Compare connection rows to eachother and smooth new one
    for (unsigned int x = 0; x < x_length; ++x) {
      // float_heights_y.at(x) = 0.0f;
      float new_height = temp_last_row_heights.at(x_length-x-1) - float_heights_y.at(x);
      float_heights_y.at(x) = new_height + float_heights_y.at(x);
    }
  }

  // Construct Heightmap
  int POSITION_DATA_SIZE_IN_ELEMENTS = 3;
  int floatsPerVertex = POSITION_DATA_SIZE_IN_ELEMENTS;

  vertices.clear();
  vertices.resize(x_length * z_length);

  int offset;

  float max_z = -FLT_MAX; // Used to calculate next_tile_start_
  // TODO min_x, max_x for turning
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length; y++) {
    for (int x = 0; x < x_length; x++) {
      offset = (y*x_length)+x;
      float xRatio = x / (float) (x_length - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length - 1));
      float yRatio = (y / (float) (z_length - 1));

      float xPosition = MIN_POSITION + (xRatio * POSITION_RANGE);
      float yPosition = float_heights_y.at(offset);
      float zPosition = MIN_POSITION + (yRatio * POSITION_RANGE);

      if (is_road) {
        vertices.at(offset) = glm::vec3(xPosition, yPosition, zPosition);
      } else {
        vertices.at(offset) = glm::vec3(xPosition + next_tile_start_.x, yPosition,
            zPosition + next_tile_start_.y);
      }
      // Calculate next_tile_start_ position
      //   Z always moves backwards
      if (zPosition + next_tile_start_.y > max_z)
        max_z = zPosition + next_tile_start_.y;
    }
  }
  if (!is_road) {
    next_tile_start_.y = max_z;
  }

  // Create Index Data
  // 2 triangles for every quad of the terrain mesh
  const unsigned int numTriangles = ( x_length - 1 ) * ( z_length - 1 ) * 2;
  // 3 indices for each triangle in the terrain mesh
  indices.clear();
  indices.resize( numTriangles * 3 );
  unsigned int index = 0; // Index in the index buffer
  for (unsigned int j = 0; j < (z_length - 1); ++j )
  {
    for (unsigned int i = 0; i < (x_length - 1); ++i )
    {
      int vertexIndex = ( j * x_length ) + i;
      // Top triangle (T0)
      indices[index++] = vertexIndex;                           // V0
      indices[index++] = vertexIndex + x_length + 1;        // V3
      indices[index++] = vertexIndex + 1;                       // V1
      // Bottom triangle (T1)
      indices[index++] = vertexIndex;                           // V0
      indices[index++] = vertexIndex + x_length;            // V2
      indices[index++] = vertexIndex + x_length + 1;        // V3
    }
  }
  if (!is_road) {
    indice_count_ = indices.size();
  } else {
    road_indice_count_ = indices.size();
  }

  // Create UV Coordinates
  texture_coordinates_uv.clear();
  texture_coordinates_uv.resize(x_length*z_length);
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length; y++) {
    for (int x = 0; x < x_length; x++) {
      offset = (y*x_length)+x;
      float xRatio = x / (float) (x_length - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length - 1));
      float yRatio = (y / (float) (z_length - 1));

      if (!is_road) {
        texture_coordinates_uv.at(offset) = glm::vec2(xRatio*float(z_length)*0.1f, yRatio*float(z_length)*0.1f);
      } else {
        texture_coordinates_uv.at(offset) = glm::vec2(xRatio, yRatio);
      }
    }
  }

  //Create Normals
  normals.clear();
  normals.resize(x_length*z_length);
  for ( unsigned int i = 0; i < indices.size(); i += 3 )  {
    glm::vec3 v0 = vertices[ indices[i + 0] ];
    glm::vec3 v1 = vertices[ indices[i + 1] ];
    glm::vec3 v2 = vertices[ indices[i + 2] ];

    glm::vec3 normal = glm::normalize( glm::cross( v1 - v0, v2 - v0 ) );

    normals[ indices[i + 0] ] += normal;
    normals[ indices[i + 1] ] += normal;
    normals[ indices[i + 2] ] += normal;
  }

  for ( unsigned int i = 0; i < normals.size(); ++i ) {
    normals[i] = glm::normalize( normals[i] );
  }

}

void Terrain::GenerateTerrainTurn(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals,
    std::vector<glm::vec2> &texture_coordinates_uv, std::vector<int> &indices, std::vector<float> &float_heights_y, const bool &is_road) {
  // Setup Vars
  // float MIN_POSITION = -10.0f;
  float MIN_POSITION = 0.0f;
  float POSITION_RANGE = 20.0f;
  int x_length = width_;
  int z_length = height_;
  if (is_road) {
    x_length = width_/8;
    z_length = height_/8;
  // MIN_POSITION = -0.35f; // Good for 0 height
  MIN_POSITION = 0.0f;
  POSITION_RANGE = 2.0f;
  }

  // Store the connecting row to smooth
  std::vector<float> temp_last_row_heights;
  for (unsigned int x = float_heights_y.size()-1; x >= float_heights_y.size()-1-x_length; --x) {
    temp_last_row_heights.push_back(float_heights_y.at(x));
  }

  float_heights_y.clear();
  // float_heights_y.resize(x_length*z_length, -0.5f);
  float_heights_y.resize(x_length*z_length, 0.15f);
  if (!is_road) {
    for (unsigned int y = 0; y < z_length; ++y) {
      for (unsigned int x = 0; x < x_length; ++x) {
        // Normalize x between -1 and 1
        float norm_x = (float)x / (x_length-1);
        norm_x *= 2.0;
        norm_x -= 1.0;

        // Height modelled using X^3
        float_heights_y.at(x+y*x_length) = 20*(norm_x*norm_x*norm_x);
      }
    }

    // Even out little section right of road
    for (unsigned int x = 0; x < z_length; ++x) {
      float_heights_y.at(x_length/2-1 + x*x_length) = 0.168f;
      float_heights_y.at(x_length/2 + x*x_length) = 0.14f;
    }

    // Randomize Top Terrain
    int center_left_x = x_length/2 - x_length/4;
    int center_z = z_length - z_length/2;
    int x_position = center_left_x, z_position = center_z;
    for (unsigned int i = 0; i < 10000; ++i) {
      int v = rand() % 4 + 1;
      switch(v) {
        case 1: x_position++;
                break;
        case 2: x_position--;
                break;
        case 3: z_position++;
                break;
        case 4: z_position--;
                break;
      }
      if (x_position < 0) {
        x_position = x_length/2-2;
        // x_position = rand() % x_length/2;
        continue;
      } else if (x_position > x_length/2-2) {
        // x_position = x_length/2;
        // x_position = rand() % x_length/2;
        x_position = 0;
        continue;
      }
      if (z_position < 0) {
        // z_position = 0;
        // z_position = rand() % z_length;
        z_position = z_length-1;
        continue;
      } else if (z_position > z_length-1) {
        // z_position = z_length-1;
        // z_position = rand() % z_length;
        z_position = 0;
        continue;
      }
      float_heights_y.at(x_position + z_position*x_length) -= 0.100f;
    }

    // Randomize Bottom Terrain
    // x_position = x_length/2, z_position = z_length-1;
    int center_right_x = x_length - x_length/2;
    x_position = center_right_x, z_position = center_z;
    for (unsigned int i = 0; i < 10000; ++i) {
      int v = rand() % 4 + 1;
      switch(v) {
        case 1: x_position++;
                break;
        case 2: x_position--;
                break;
        case 3: z_position++;
                break;
        case 4: z_position--;
                break;
      }
      if (x_position < x_length/2+4) {
        // x_position = x_length/2;
        // x_position = rand() % x_length/2 + x_length/2;
        x_position = x_length-1;
        continue;
      } else if (x_position > x_length-1) {
        // x_position = x_length-1;
        // x_position = rand() % x_length/2 + x_length/2;
        x_position = x_length/2+4;
        continue;
      }
      if (z_position < 0) {
        // z_position = 0;
        // z_position = rand() % z_length;
        z_position = z_length-1;
        continue;
      } else if (z_position > z_length-1) {
        // z_position = z_length-1;
        // z_position = rand() % z_length;
        z_position = 0;
        continue;
      }
      float_heights_y.at(x_position + z_position*x_length) += 0.100f;
    }

    // TODO someone try fighting with this if you dare...
    //   Something goes wrong with the normals at the connection
    // Compare connection rows to eachother and smooth new one
    for (unsigned int x = 0; x < x_length; ++x) {
      // float_heights_y.at(x) = 0.0f;
      float new_height = temp_last_row_heights.at(x_length-x-1) - float_heights_y.at(x);
      float_heights_y.at(x) = new_height + float_heights_y.at(x);
    }
  }

  // Construct Heightmap
  int POSITION_DATA_SIZE_IN_ELEMENTS = 3;
  int floatsPerVertex = POSITION_DATA_SIZE_IN_ELEMENTS;

  vertices.clear();
  vertices.resize(x_length * z_length);

  int offset;

  float max_z = -FLT_MAX; // Used to calculate next_tile_start_
  // TODO min_x, max_x for turning
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length; y++) {
    for (int x = 0; x < x_length; x++) {
      offset = (y*x_length)+x;
      float xRatio = x / (float) (x_length - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length - 1));
      float yRatio = (y / (float) (z_length - 1));

      float xPosition = MIN_POSITION + (xRatio * POSITION_RANGE);
      float yPosition = float_heights_y.at(offset);
      // float zPosition = MIN_POSITION + (yRatio * yRatio * 10 * POSITION_RANGE);
      float zPosition = MIN_POSITION + (yRatio * POSITION_RANGE);

      float zSquare = zPosition * zPosition; //x^2

      xPosition = zSquare/40 + xPosition;
      // xPosition = zPosition + xPosition;

      // xPosition = zPosition + xPosition;

      if (is_road) {
        vertices.at(offset) = glm::vec3(xPosition, yPosition, zPosition);
      } else {
        vertices.at(offset) = glm::vec3(xPosition + next_tile_start_.x, yPosition,
            zPosition + next_tile_start_.y);
      }
      // Calculate next_tile_start_ position
      //   Z always moves backwards
      if (zPosition + next_tile_start_.y > max_z)
        max_z = zPosition + next_tile_start_.y;
    }
  }
  if (!is_road) {
    next_tile_start_.y = max_z;
  }

  // Create Index Data
  // 2 triangles for every quad of the terrain mesh
  const unsigned int numTriangles = ( x_length - 1 ) * ( z_length - 1 ) * 2;
  // 3 indices for each triangle in the terrain mesh
  indices.clear();
  indices.resize( numTriangles * 3 );
  unsigned int index = 0; // Index in the index buffer
  for (unsigned int j = 0; j < (z_length - 1); ++j )
  {
    for (unsigned int i = 0; i < (x_length - 1); ++i )
    {
      int vertexIndex = ( j * x_length ) + i;
      // Top triangle (T0)
      indices[index++] = vertexIndex;                           // V0
      indices[index++] = vertexIndex + x_length + 1;        // V3
      indices[index++] = vertexIndex + 1;                       // V1
      // Bottom triangle (T1)
      indices[index++] = vertexIndex;                           // V0
      indices[index++] = vertexIndex + x_length;            // V2
      indices[index++] = vertexIndex + x_length + 1;        // V3
    }
  }
  if (!is_road) {
    indice_count_ = indices.size();
  } else {
    road_indice_count_ = indices.size();
  }

  // Create UV Coordinates
  texture_coordinates_uv.clear();
  texture_coordinates_uv.resize(x_length*z_length);
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length; y++) {
    for (int x = 0; x < x_length; x++) {
      offset = (y*x_length)+x;
      float xRatio = x / (float) (x_length - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length - 1));
      float yRatio = (y / (float) (z_length - 1));

      if (!is_road) {
        texture_coordinates_uv.at(offset) = glm::vec2(xRatio*float(z_length)*0.1f, yRatio*float(z_length)*0.1f);
      } else {
        texture_coordinates_uv.at(offset) = glm::vec2(xRatio, yRatio);
      }
    }
  }

  //Create Normals
  normals.clear();
  normals.resize(x_length*z_length);
  for ( unsigned int i = 0; i < indices.size(); i += 3 )  {
    glm::vec3 v0 = vertices[ indices[i + 0] ];
    glm::vec3 v1 = vertices[ indices[i + 1] ];
    glm::vec3 v2 = vertices[ indices[i + 2] ];

    glm::vec3 normal = glm::normalize( glm::cross( v1 - v0, v2 - v0 ) );

    normals[ indices[i + 0] ] += normal;
    normals[ indices[i + 1] ] += normal;
    normals[ indices[i + 2] ] += normal;
  }

  for ( unsigned int i = 0; i < normals.size(); ++i ) {
    normals[i] = glm::normalize( normals[i] );
  }

}

unsigned int Terrain::CreateVao(const GLuint &program_id, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
    const std::vector<glm::vec2> &texture_coordinates_uv, const std::vector<int> &indices) {

  //////////////////////////////
  //Create axis VAO         ////
  //////////////////////////////
  glUseProgram(terrain_program_id_);

  assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3); //Vec3 cannot be loaded to buffer this way

  unsigned int VAO_handle;
  glGenVertexArrays(1, &VAO_handle);
  glBindVertexArray(VAO_handle);

  int vertLoc = glGetAttribLocation(program_id, "a_vertex");
  int normLoc = glGetAttribLocation(program_id, "a_normal");
  int textureLoc = glGetAttribLocation(program_id, "a_texture");

  // Buffers to store position, colour and index data
  unsigned int buffer[4];
  glGenBuffers(4, buffer);

  // Set vertex position
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, 
      sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
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
      sizeof(int)*indices.size(), &indices[0], GL_STATIC_DRAW);   
  // sizeof(iIndices), iIndices, GL_STATIC_DRAW);   
  // Un-bind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return VAO_handle;
}

// Creates a texture pointer from file
//   @return new_texture, a GLuint texture pointer
GLuint Terrain::LoadTexture(const std::string &filename) {
  // A shader program has many texture units, slots in which a texture can be bound, available to
  // it and this function defines which unit we are working with currently
  // We will only use unit 0 until later in the course. This is the default.
  glActiveTexture(GL_TEXTURE0);

  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  GLuint new_texture;
  glGenTextures( 1, &new_texture );

  // load an image from file as texture 1
  int x, y, n;
  unsigned char *data;
  data = stbi_load(
      filename.c_str(), /*char* filepath */
      // "crate.jpg",
      &x, /*The address to store the width of the image*/
      &y, /*The address to store the height of the image*/
      &n  /*Number of channels in the image*/,
      0   /*Force number of channels if > 0*/
      );

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

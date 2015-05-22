#include "terrain.h"

#include "glm/gtx/rotate_vector.hpp"

Terrain::Terrain(const GLuint &program_id, const GLuint &water_id, const int &width, const int &height)
  : x_length_(width), z_length_(height),
  indice_count_(0), road_indice_count_(0),
  terrain_program_id_(program_id), terrain_water_id_(water_id),
  rotation_(0) {
    // New Seed
    srand(time(NULL));
    // Setup Vars
    heights_.resize(x_length_ * z_length_, 0.0f); // Magic number needs to be the same as inside the functions
    next_tile_start_ = glm::vec2(-10,-10);
    prev_max_x_ = 20.0f; // DONT TOUCH - Magic number derived from min_position

    // Textures
    texture_ = LoadTexture("textures/rock01.jpg");
    road_texture_ = LoadTexture("textures/road.jpg");
    water_texture_ = LoadTexture("textures/water01.jpg");

    // Setup Indices and UV Coordinates
    //   These never change unless the x_length_ and/or z_length_ of the heightmap change
    HelperMakeIndicesAndUV();
    indice_count_ = indices.size();
    indice_count_water_ = indices.size();
    HelperMakeRoadIndicesAndUV(); // BEWARD FULL OF MAGIC NUMBERS
    road_indice_count_ = indices_road_.size();

    // Generate Starting Terrain
    // This is the amount of tiles that will be in the circular_vector at all times
    // Always start with 2 straight pieces so car is on 2nd tile road
    //   and so can't see first tile being popped off
    GenerateTerrain();
    // Water needs indices and UV coordinates from Terrain generation.
    //   i.e. needs to be called succeeding an initial Terrain.
    GenerateWater();
    // Pop off first collision map which is already behind car
    col_pop();
    GenerateTerrain();
    for (int x = 0; x < 5; ++x) {
      // Generates a random terrain piece and pushes it back
      // into circular_vector VAO buffer
      RandomizeGeneration();
      // GenerateTerrain();
    }
  }

// Generates next tile and removes first one
//   Uses the circular_vector data structure to do this in O(1)
//   TODO merge col_pop or something
void Terrain::ProceedTiles() {
  // TODO free/delete GL VAOs
  terrain_vao_handle_.pop_front();
  road_vao_handle_.pop_front();

  // Generates a random terrain piece and pushes it back
  // into circular_vector VAO buffer
      RandomizeGeneration();
      // GenerateTerrain();
}

// Generates a random terrain piece and pushes it back into circular_vector VAO buffer
void Terrain::RandomizeGeneration() {
  int v = rand() % 2;
  switch(v) {
    case 0:
      GenerateTerrain();
      break;
    case 1:
      GenerateTerrainTurn();
      break;
  }
}

// Straight Terrain Piece
//   Mutates the input members, (e.g. vertices, indicies etc.) and then
//   calls CreateVAO and pushes the result back to the terrain_vao_handle_
//   @warn creates and pushes back a road VAO based on the terrain middle section
//   @warn pushes next road collision map into member queue
void Terrain::GenerateTerrain() {
  HelperMakeHeights();
  HelperMakeVertices(kStraight);
  HelperMakeNormals();
  unsigned int terrain_vao = CreateVao(kTerrain);
  terrain_vao_handle_.push_back(terrain_vao);

  //  ROAD - Extract middle flat section and make road VAO
  //  BEWARD FULL OF MAGIC NUMBERS
  HelperMakeRoadVertices();
  //  Road Normals only have to be generated once
  //    because the surface is relatively flat
  if (normals_road_.size() == 0)
    HelperMakeRoadNormals();
  
  // Collision map for current road tile
  HelperMakeRoadCollisionMap();

  unsigned int road_vao = CreateVao(kRoad);
  road_vao_handle_.push_back(road_vao);

}

// Turning Terrain Piece
//   Mutates the input members, (e.g. vertices, indicies etc.) and then
//   calls CreateVAO and pushes the result back to the terrain_vao_handle_
//   @warn creates and pushes back a road VAO based on the terrain middle section
//   @warn pushes next road collision map into member queue
void Terrain::GenerateTerrainTurn() {
  HelperMakeHeights();
  HelperMakeVertices(kTurnLeft);
  HelperMakeNormals();
  unsigned int terrain_vao = CreateVao(kTerrain);
  terrain_vao_handle_.push_back(terrain_vao);

  //  ROAD - Extract middle flat section and make road VAO
  //  BEWARD FULL OF MAGIC NUMBERS
  HelperMakeRoadVertices();
  //  Road Normals only have to be generated once
  //    because the surface is relatively flat.
  //  Already done in straight road piece (i.e. starting piece)
  // if (normals_road_.size() == 0)
  //   HelperMakeRoadNormals();

  // Collision map for current road tile
  HelperMakeRoadCollisionMap();

  unsigned int road_vao = CreateVao(kRoad);
  road_vao_handle_.push_back(road_vao);

}

// Water height map
//   Mutates all the input parameters for CreateVAO
//   @warn creates and pushes back a water VAO
//   @warn uses the indices and UV coordinates from terrain generation
//         hence must be called after GenerateTerrain()
// TODO if this becomes another dynamic tile then dont regenerate height vector (its expensive)
void Terrain::GenerateWater() {
  // Setup Vars
  // float min_position = -10.0f;
  float min_position = 0.0f;
  float position_range = 50.0f;

  // (COMMENTED OUT) I don't think this entire block actually does anything?
  //
  // water_heights_.clear();
  // water_heights_.resize(x_length_*z_length_, 0.0f);
  //
  // // MAKE FLAT WATER HEIGHTMAP
  // int x_position = 0, z_position = 0;
  // for (unsigned int i = 0; i < 10000; ++i) {
  //   int v = rand() % 4 + 1;
  //   switch(v) {
  //     case 1: x_position++;
  //             break;
  //     case 2: x_position--;
  //             break;
  //     case 3: z_position++;
  //             break;
  //     case 4: z_position--;
  //             break;
  //   }
  //   if (x_position < 0) {
  //     x_position = x_length_-1;
  //     continue;
  //   } else if (x_position > x_length_-1) {
  //     x_position = 0;
  //     continue;
  //   }
  //   if (z_position < 0) {
  //     z_position = z_length_-1;
  //     continue;
  //   } else if (z_position > z_length_-1) {
  //     z_position = 0;
  //     continue;
  //   }
  //   water_heights_.at(x_position + z_position*x_length_) += 0.100f;
  // }

  water_heights_.assign(x_length_*z_length_, 0.0f);

  HelperMakeVertices(kStraight, kWater, min_position, position_range);
  // TODO normal recalculation?
  //   This Helper will only create them for a flat surface
  HelperMakeNormals();
  unsigned int water_vao = CreateVao(kWater);
  water_vao_handle_ = water_vao;
}

// Model the heights using an X^3 mathematical functions, then randomize heights
// for all vertices in heightmap
//   @warn pretty expensive operation 10000*2 loops
void Terrain::HelperMakeHeights() {
  // Store the connecting row to smooth
  std::vector<float> temp_last_row_heights_;
  for (unsigned int x = heights_.size()-1; x >= heights_.size()-1-x_length_; --x) {
    temp_last_row_heights_.push_back(heights_.at(x));
  }

  heights_.assign(x_length_*z_length_, 0.00f); // 2nd param is default height
  for (int y = 0; y < z_length_; ++y) {
    for (int x = 0; x < x_length_; ++x) {
      // Normalize x between -1 and 1
      float norm_x = (float)x / (x_length_-1);
      norm_x *= 2.0;
      norm_x -= 1.0;

      // Height modelled using X^3
      heights_.at(x+y*x_length_) = 20*(norm_x*norm_x*norm_x);
    }
  }

  // Randomize Top Terrain
  int center_left_x = x_length_/2 - x_length_/4;
  int center_z = z_length_ - z_length_/2;
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
      x_position = x_length_/2-2;
      // x_position = rand() % x_length_/2;
      continue;
    } else if (x_position > x_length_/2-2) {
      // x_position = x_length_/2;
      // x_position = rand() % x_length_/2;
      x_position = 0;
      continue;
    }
    if (z_position < 0) {
      // z_position = 0;
      // z_position = rand() % z_length_;
      z_position = z_length_-1;
      continue;
    } else if (z_position > z_length_-1) {
      // z_position = z_length_-1;
      // z_position = rand() % z_length_;
      z_position = 0;
      continue;
    }
    heights_.at(x_position + z_position*x_length_) -= 0.100f;
  }

  // Randomize Bottom Terrain
  // x_position = x_length_/2, z_position = z_length_-1;
  int center_right_x = x_length_ - x_length_/2;
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
    if (x_position < x_length_/2+4) {
      // x_position = x_length_/2;
      // x_position = rand() % x_length_/2 + x_length_/2;
      x_position = x_length_-1;
      continue;
    } else if (x_position > x_length_-1) {
      // x_position = x_length_-1;
      // x_position = rand() % x_length_/2 + x_length_/2;
      x_position = x_length_/2+4;
      continue;
    }
    if (z_position < 0) {
      // z_position = 0;
      // z_position = rand() % z_length_;
      z_position = z_length_-1;
      continue;
    } else if (z_position > z_length_-1) {
      // z_position = z_length_-1;
      // z_position = rand() % z_length_;
      z_position = 0;
      continue;
    }
    heights_.at(x_position + z_position*x_length_) += 0.100f;
  }

  // SMOOTH CONNECTIONS
  // TODO someone try fighting with this if you dare...
  //   Something goes wrong with the normals at the connection
  // Compare connection rows to eachother and smooth new one
  for (int x = 0; x < x_length_; ++x) {
    // heights_.at(x) = 0.0f;
    float new_height = temp_last_row_heights_.at(x_length_-x-1) - heights_.at(x);
    heights_.at(x) = new_height + heights_.at(x);
  }
}

// Overloaded function to generate a square height map on the X/Z plane. Different
// road_type parameters can be added to curve the Z coordinates and hence make turning
// pieces.
// @param  road_type       An enum representing the mathematical model to be applied to Z
// @param  tile_type       An enum representing whether the tile is water or terrain
// @param  min_position    The relative start position of the heightmap over X/Z
// @param  position_range  The spread of the heightmap over X/Z 
// @warn  No changes can be made to vertices_ member until the Road Helpers complete
void Terrain::HelperMakeVertices(RoadType road_type, TileType tile_type,
    float min_position, float position_range) {
  vertices.assign(x_length_ * z_length_, glm::vec3());

  int offset;
  float max_z = -FLT_MAX; // Used to calculate next_tile_start_
  // TODO min_x, max_x for turning
  float max_x = -FLT_MAX;
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length_; y++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (y*x_length_)+x;
      float xRatio = x / (float) (x_length_ - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length_ - 1));
      float yRatio = (y / (float) (z_length_ - 1));

      float xPosition = min_position + (xRatio * position_range);
      float yPosition;
      float zPosition = min_position + (yRatio * position_range);

      // Water or Terrain
      switch(tile_type) {
        case kTerrain:
          yPosition = heights_.at(offset);
          break;
        case kWater:
          yPosition = water_heights_.at(offset);
          break;
        case kRoad:
          assert(0 && "improper use of function");
          break;
      }

      // Curve addition
      switch(road_type) {
        // case 0: straight road => do nothing
        // case 1: x^2 turnning road
        case kTurnLeft:
          float zSquare = zPosition * zPosition; //x^2
          xPosition = zSquare/(position_range*5.5) + xPosition;
          break;
      }

      vertices.at(offset) = glm::vec3(xPosition, yPosition, zPosition);
    }
  }
  // special point for finding pivot translation
  glm::vec3 &pivot = vertices.at(20*x_length_/36 );
  // pivot.y = 1000000;
  // normals_road_.push_back(normals.at(x + z*x_length_));
  glm::vec3 foo = glm::rotateY(pivot, rotation_);
  float translate_x = pivot.x - foo.x;
  float translate_z = pivot.z - foo.z;
  for (int y = 0; y < z_length_; y++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (y*x_length_)+x;

      glm::vec3 foo = vertices.at(offset);
      // foo = glm::rotate(foo, 30.0f, glm::vec3(next_tile_start_.x,1,next_tile_start_.y));
      foo = glm::rotateY(foo, rotation_);
      // foo = foo - ( glm::vec3(next_tile_start_.x, 0, next_tile_start_.y));
      float xPosition = foo.x + translate_x;
      float zPosition = foo.z + translate_z;
      // xPosition = foo.x + position_range/2;
      float yPosition = foo.y;

      // vertices.at(offset) = glm::vec3(xPosition + next_tile_start_.x, yPosition,
      //     zPosition + next_tile_start_.y);
      vertices.at(offset) = glm::vec3(xPosition + next_tile_start_.x, yPosition,
          zPosition + next_tile_start_.y);

      // Calculate next_tile_start_ position
      //   Z always moves backwards
      if (zPosition + next_tile_start_.y > max_z)
        max_z = zPosition + next_tile_start_.y;
      // Calulate next_tile_start_x position
      //   X moves left and right
      if (xPosition > max_x)
        // max_x = xPosition + next_tile_start_.x;
        max_x = xPosition;
    }
  }
  // Water or Terrain
  switch(tile_type) {
    case kTerrain:
      const glm::vec3 &pivot_end = vertices.at(20*x_length_/36 + (z_length_-2)*x_length_);
      // Set next z position
      next_tile_start_.y = max_z;
      next_tile_start_.y = pivot_end.z;

      // max_x = pivot.x;
      // Calculate next_tile_start_.x position (next X tile position)
      float displacement_x = max_x - prev_max_x_;
      prev_max_x_ = max_x - displacement_x;

      displacement_x = pivot_end.x - pivot.x;
      next_tile_start_.x += displacement_x;
      // next_tile_start_.x = position_range/2 - pivot.x;
      break;
  }
  switch(road_type) {
    case kTurnLeft:
      rotation_ += 20;
      // rotation_ = 0;
      break;
  }
}

// Generates the indices and UV texture coordinates to be used by the tile
// @note  These don't change for the same x_length_ * z_length_ height maps
// @warn  No changes can be made to indices or UV member until the Road Helpers complete
void Terrain::HelperMakeIndicesAndUV() {
  // CONSTRUCT HEIGHT MAP INDICES
  // 2 triangles for every quad of the terrain mesh
  const unsigned int numTriangles = ( x_length_ - 1 ) * ( z_length_ - 1 ) * 2;
  // 3 indices for each triangle in the terrain mesh
  indices.assign( numTriangles * 3 , int() );
  unsigned int index = 0; // Index in the index buffer
  for (unsigned int j = 0; j < (z_length_ - 1); ++j )
  {
    for (unsigned int i = 0; i < (x_length_ - 1); ++i )
    {
      int vertexIndex = ( j * x_length_ ) + i;
      // Top triangle (T0)
      indices[index++] = vertexIndex;                        // V0
      indices[index++] = vertexIndex + x_length_ + 1;        // V3
      indices[index++] = vertexIndex + 1;                    // V1
      // Bottom triangle (T1)
      indices[index++] = vertexIndex;                        // V0
      indices[index++] = vertexIndex + x_length_;            // V2
      indices[index++] = vertexIndex + x_length_ + 1;        // V3
    }
  }

  // CONSTRUCT UV COORDINATES
  texture_coordinates_uv.assign(x_length_*z_length_, glm::vec2());
  int offset;
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length_; y++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (y*x_length_)+x;
      float xRatio = x / (float) (x_length_ - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      // float yRatio = 1.0f - (y / (float) (z_length_ - 1));
      float yRatio = (y / (float) (z_length_ - 1));

      texture_coordinates_uv.at(offset) = glm::vec2(xRatio*float(z_length_)*0.1f, yRatio*float(z_length_)*0.1f);
    }
  }
}

// Generates the normals by doing a cross product of neighbouring vertices
// @warn  No changes can be made to normals_ member until the Road Helpers complete
void Terrain::HelperMakeNormals() {
  normals.assign(x_length_*z_length_, glm::vec3());
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

// Rip the road parts of the terrain heightmap using calulcated magic numbers and store
// these in the vertices_road_ vector
// @warn  requires a preceeding call to HelperMakeVertices otherwise undefined behaviour
void Terrain::HelperMakeRoadVertices() {
  vertices_road_.clear();
  for (unsigned int x = 17*x_length_/36; x < 22*x_length_/36; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z){
      vertices_road_.push_back(vertices.at(x + z*x_length_));
      // Lift road a bit above terrain to make it visible
      vertices_road_.back().y += 0.01f;
    }
  }
}

// Rip the road parts of the terrain normals vector using calulcated magic numbers and store
// these in the normals_road_ vector
// @warn  requires a preceeding call to HelperMakeNormals otherwise undefined behaviour
// @warn  for optimzation this should only be called once because road normals don't change
void Terrain::HelperMakeRoadNormals() {
  normals_road_.clear();
  for (unsigned int x = 17*x_length_/36; x < 22*x_length_/36; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z){
      normals_road_.push_back(normals.at(x + z*x_length_));
    }
  }
}

// Rip the road parts of the terrain indice and UV vectors using calulcated magic numbers and 
// store these in the indice and UV vectors respectfully
// @warn  requires a preceeding call to HelperMakeIndicesAndUV otherwise undefined behaviour
// @warn  for optimzation this should only be called once because road indices and UV don't change
void Terrain::HelperMakeRoadIndicesAndUV() {
  // Create Index Data
  indices_road_.clear();
  for (unsigned int j = 0; j < (4*z_length_/36 - 0); ++j )
  {
    for (unsigned int i = 0; i < (x_length_ - 1); ++i )
    {
      int vertexIndex = ( j * x_length_ ) + i;
      // Top triangle (T0)
      indices_road_.push_back(vertexIndex);                        // V0
      indices_road_.push_back(vertexIndex + x_length_ + 1);        // V3
      indices_road_.push_back(vertexIndex + 1);                    // V1 visualization
      // Bottom triangle (T1)
      indices_road_.push_back(vertexIndex);                        // V0
      indices_road_.push_back(vertexIndex + x_length_);            // V2
      indices_road_.push_back(vertexIndex + x_length_ + 1);        // V3
    }
  }

  // Create UV Data
  texture_coordinates_uv_road_.clear();
  for (unsigned int x = 0; x < 5*x_length_/36; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z){
      // The multiplications below change stretch of the texture (ie repeats)
      texture_coordinates_uv_road_.push_back(glm::vec2(
            texture_coordinates_uv.at(x + z*x_length_).x * 3.2,
            texture_coordinates_uv.at(x + z*x_length_).y * 1));
    }
  }
}

// Generates a collision coordinate mapping
//   Using the vertices_road_ vector, maps all rounded Z coordinates to their corresponding min X
//   and Max X coordinates. Then pushes the map into the member queue ready for collision checking.
// @warn  requires a preceeding call to HelperMakeRoadVertices otherwise undefined behaviour
void Terrain::HelperMakeRoadCollisionMap() {
  std::vector<std::pair<glm::vec3,glm::vec3>> tile_map;
  tile_map.reserve(z_length_);
  std::pair<glm::vec3,glm::vec3> min_max_x_pair;
  unsigned int x_new_row_size = 22*x_length_/36 - 17*x_length_/36;

  for (unsigned int z = 0; z < z_length_; ++z){
    float z_key = vertices_road_.at(0 + z).z; // z coordinate of first vertice in row
    // z_key = round(z_key); // round value so key can be found
    min_max_x_pair.first = vertices_road_.at(0 + z); // left? side vertices
    min_max_x_pair.second = vertices_road_.at(z + z_length_ * (x_new_row_size - 1)); // other side vertices

    // vertices_road_.at(0+z).y = 10000; //minx visualization
    // vertices_road_.at(z + z_length_ * (x_new_row_size - 1)).y = 10000; //maxx visualization

    tile_map.push_back(min_max_x_pair); // doesnt insert when duplicate
    // tile_map[z_key] = min_max_x_pair; // overrides duplicates


    // MAPPING DEBUGS
    // // printf("test = %f, z = %f\n", vertices_top.at(z).x, z_key);
    // printf("z = %f\n", z_key);
    // printf("min x = %f, max x = %f\n", min_max_x_pair.first, min_max_x_pair.second);
  }

  collision_queue_hash_.push(tile_map);
  // for (auto& x: collision_queue_hash_.front()) {
  //   printf("hashed z = %f, x_min = %f, x_max = %f\n", x.first, x.second.first, x.second.second);
  // }
}

// Creates a new vertex array object and loads in data into a vertex attribute buffer
//   The parameters are self explanatory.
//   @return vao_handle, the vao handle
unsigned int Terrain::CreateVao(const GLuint &program_id, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
    const std::vector<glm::vec2> &texture_coordinates_uv, const std::vector<int> &indices) {

  glUseProgram(program_id);

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

// Creates a new vertex array object and loads in data into a vertex attribute buffer
//   @param  tile_type  An enum representing the proper members to use
//   @return vao_handle, the vao handle
unsigned int Terrain::CreateVao(TileType tile_type) {
  unsigned int vao;
  switch(tile_type) {
    case kTerrain: //and kRoad
      vao = CreateVao(terrain_program_id(), vertices, normals, texture_coordinates_uv, indices);
      break;
    case kWater:
      // TODO fix this nonconst direct member access
      vao = CreateVao(terrain_water_id_, vertices, normals, texture_coordinates_uv, indices);
      break;
    case kRoad:
      vao = CreateVao(terrain_program_id(), vertices_road_, normals_road_, texture_coordinates_uv_road_, indices_road_);
      break;
  }
  return vao;
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

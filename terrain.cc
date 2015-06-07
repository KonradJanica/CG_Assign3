#include "terrain.h"

#include "glm/gtx/rotate_vector.hpp"

Terrain::Terrain(const GLuint program_id, const int &width, const int &height)
  : x_length_(width), z_length_(height), length_multiplier_(width / 32), kRandomIterations(10000*length_multiplier_),
  generated_ticks_(0), prev_rand_(0), prev_cliff_x3_rand_(rand() % 20 + 1), prev_water_x3_rand_(rand() % 15 + 5), prev_spacing_rand_((rand() % 100)*0.003f - 0.15f),
  indice_count_(0), road_indice_count_(0), terrain_program_id_(program_id),
  rotation_(0), prev_rotation_(0), z_smooth_max_(10 * length_multiplier_) {
    // New Seed
    srand(time(NULL));
    // Setup Vars
    heights_.resize(x_length_ * z_length_); // Initialize to 0 to avoid seg fault during smoothing
    vertices.resize(x_length_ * z_length_); // Initialize to 0 to avoid seg fault during smoothing
    normals.resize(x_length_ * z_length_);  // Initialize to 0 to avoid seg fault during smoothing
    next_tile_start_ = glm::vec2(-10,-10);
    prev_max_x_ = 20.0f; // DONT TOUCH - Magic number derived from min_position
    // Height randomization vars
    int center_left_x = x_length_/2 - x_length_/4;
    int center_z = z_length_ - z_length_/2;
    x_water_position_ = center_left_x, z_water_position_ = center_z;
    int center_right_x = x_length_ - x_length_/2;
    x_cliff_position_ = center_right_x, z_cliff_position_ = center_z;
    // Reserve space (required to ensure default iterators are not invalidated)
    collision_queue_hash_.reserve(6);

    // Textures
    glActiveTexture(GL_TEXTURE0);
    texture_ = LoadTexture("textures/rock01.jpg");
    road_texture_ = LoadTexture("textures/road.jpg");

    glActiveTexture(GL_TEXTURE1);


    cliff_nrm_texture_ = LoadTexture("textures/normalMap.tga");


    // Setup Indices and UV Coordinates
    //   These never change unless the x_length_ and/or z_length_ of the heightmap change
    HelperMakeIndicesAndUV();
    indice_count_ = indices.size();

    HelperMakeRoadIndicesAndUV(); // BEWARD FULL OF MAGIC NUMBERS
    road_indice_count_ = indices_road_.size();
    //  Road Normals only have to be generated once
    //    because the surface is relatively flat
    HelperMakeRoadNormals();

    // Generate Starting Terrain
    // This is the amount of tiles that will be in the circular_vector at all times
    // Always start with 3 straight pieces so car is on 3rd tile road
    //   and so can't see first tile being popped off
    GenerateStartingTerrain(kStraight);
    GenerateStartingTerrain(kStraight);
    GenerateStartingTerrain(kStraight);

    for (int x = 0; x < 5; ++x) {
      // Generates a random terrain piece and pushes it back
      // into circular_vector VAO buffer
      RandomizeGeneration(true);
    }

    // Pop off first and second collision map which is already behind car
    col_pop();
    col_pop();
  }

// Generates next tile and removes first one
//   Uses the circular_vector data structure to do this in O(1)
//   Resets the generation ticks to 0, E.g. begins generating next tile
//   TODO merge col_pop or something
void Terrain::ProceedTiles() {
  // TODO free/delete GL VAOs
  // glDeleteVertexArray(1, &terrain_vao_handle_[0]);
  // glDeleteVertexArray(1, &road_vao_handle_[0]);
  terrain_vao_handle_.pop_front();
  road_vao_handle_.pop_front();

  // Reset generation state
  generated_ticks_ = 0;

  // Generates a random terrain piece and pushes it back
  // into circular_vector VAO buffer
  // TODO add different chances to prev_rand_
  prev_rand_ = rand() % 3;
  z_smooth_max_ = (rand() % 3 + 8)*length_multiplier_;
  RandomizeGeneration();
}

// Generates the next part of tile for spreading over multiple ticks
void Terrain::GenerationTick() {
  //Check for overflow
  if (generated_ticks_ >= 0) {
    // printf("gen ticks = %d\n",generated_ticks_);
    ++generated_ticks_;
    RandomizeGeneration();
  }
}

// Generates a random terrain piece and pushes it back into circular_vector VAO buffer
void Terrain::RandomizeGeneration(const bool is_start) {
  if (is_start) {
    switch(prev_rand_) {
      case 0:
        GenerateStartingTerrain(kStraight);
        break;
      case 1:
        GenerateStartingTerrain(kTurnLeft);
        break;
      case 2:
        GenerateStartingTerrain(kTurnRight);
        break;
    }
  } else {
    switch(prev_rand_) {
      case 0:
        GenerateTerrain(kStraight);
        break;
      case 1:
        GenerateTerrain(kTurnLeft);
        break;
      case 2:
        GenerateTerrain(kTurnRight);
        break;
    }
  }
}

// Generate Terrain tile piece with road
//   Mutates the input members, (e.g. vertices, indicies etc.) and then
//   calls CreateVAO and pushes the result back to the terrain_vao_handle_
//   @param The tile type to generate e.g. kStraight, kTurnLeft etc.
//   @warn creates and pushes back a road VAO based on the terrain middle section
//   @warn pushes next road collision map into member queue
void Terrain::GenerateStartingTerrain(RoadType road_type) {
  HelperMakeHeights(0, kRandomIterations);
  HelperMakeSmoothHeights(true);
  HelperMakeSmoothHeights(false); //load is spread over 2 ticks after start
  // Expand spacing of tiles subtly
  float v = (rand() % 100)*0.003f - 0.15f;
  // printf("v = %f\n",v);
  prev_spacing_rand_ += v;
  if (prev_spacing_rand_ < 20)
    prev_spacing_rand_ = 20;
  else if (prev_spacing_rand_ > 25)
    prev_spacing_rand_ = 25;
  HelperMakeVertices(road_type, kTerrain, 0, prev_spacing_rand_);
  HelperMakeNormals();
  //  ROAD - Extract middle flat section and make road VAO
  //  BEWARD FULL OF MAGIC NUMBERS
  HelperMakeRoadVertices();
  // Fix the UV caused by z_smooth_max_ being random
  // HelperFixUV();
  // Collision map for current road tile
  HelperMakeRoadCollisionMap();
  // Make VAOs
  unsigned int terrain_vao = CreateVao(kTerrain);
  terrain_vao_handle_.push_back(terrain_vao);
  unsigned int road_vao = CreateVao(kRoad);
  road_vao_handle_.push_back(road_vao);

}

// Generate Terrain tile piece with road
//   Mutates the input members, (e.g. vertices, indicies etc.) and then
//   calls CreateVAO and pushes the result back to the terrain_vao_handle_
//   The members are mutated over $kGenerationTicks to spread load
//   @param The tile type to generate e.g. kStraight, kTurnLeft etc.
//   @warn creates and pushes back a road VAO based on the terrain middle section
//   @warn pushes next road collision map into member queue
void Terrain::GenerateTerrain(RoadType road_type) {
  if (generated_ticks_ < kHeightGenerationTicks
      && generated_ticks_ >= 0) {
    int iterations_per_tick = kRandomIterations/kHeightGenerationTicks;
    int start = iterations_per_tick * generated_ticks_;
    int end = start + iterations_per_tick;
    // printf("start = %d, end = %d\n",start,end);
    HelperMakeHeights(start, end);

    return;
  }
  if (generated_ticks_ == kHeightGenerationTicks) {
    HelperMakeSmoothHeights(true);

    return;
  }

  // The amount of ticks after heights have been generated
  signed char vao_ticks = generated_ticks_ - kHeightGenerationTicks;

  switch(vao_ticks) {
    case 1:
      HelperMakeSmoothHeights(false);
      break;
    case 2:
      {
        // Expand spacing of tiles subtly
        float v = (rand() % 100)*0.003f - 0.15f;
        // printf("v = %f\n",v);
        prev_spacing_rand_ += v;
        if (prev_spacing_rand_ < 20)
          prev_spacing_rand_ = 20;
        else if (prev_spacing_rand_ > 25)
          prev_spacing_rand_ = 25;
        HelperMakeVertices(road_type, kTerrain, 0, prev_spacing_rand_);
        break;
      }
    case 3:
      HelperMakeNormals();
      break;
    case 4:
      //  ROAD - Extract middle flat section and make road VAO
      //  BEWARD FULL OF MAGIC NUMBERS
      HelperMakeRoadVertices();
      break;
    case 5:
      // Collision map for current road tile
      HelperMakeRoadCollisionMap();
      break;
    case 6:
      {
        // Make terrain VAO
        unsigned int terrain_vao = CreateVao(kTerrain);
        terrain_vao_handle_.push_back(terrain_vao);
        break;
      }
    case 7:
      {
        // Make road VAO
        unsigned int road_vao = CreateVao(kRoad);
        road_vao_handle_.push_back(road_vao);
        break;
      }
  }
}

// Model the heights using an X^3 mathematical functions, then randomize heights
// for all vertices in heightmap
//   @param  start  Index to start looping from
//   @param  end    Index to finish the loop
//   @warn pretty expensive operation 10000*2 loops
//   @warn spread over a couple of loops
void Terrain::HelperMakeHeights(const int start, const int end) {
  if (generated_ticks_ == 0) {

    // Store the connecting row to smooth
    temp_last_row_heights_.assign(heights_.end()-x_length_, heights_.end());

    // Generate base model of terrain (X^3 i.e. cubic)
    prev_cliff_x3_rand_ += rand() % 8 - 4; //flucuation of the cliff base height
    if (prev_cliff_x3_rand_ < 5)
      prev_cliff_x3_rand_ = 5;
    else if (prev_cliff_x3_rand_ > 20)
      prev_cliff_x3_rand_ = 20;
    prev_water_x3_rand_ += rand() % 6 - 3; //flucuation of the water base height
    if (prev_water_x3_rand_ < 5)
      prev_water_x3_rand_ = 5;
    else if (prev_water_x3_rand_ > 20)
      prev_water_x3_rand_ = 20;
    for (int z = 0; z < z_length_; ++z) {
      for (int x = 0; x < x_length_; ++x) {
        // Normalize x between -1 and 1
        float norm_x = (float)x / (x_length_-1);
        norm_x *= 2.0;
        norm_x -= 1.0;

        // Height modelled using X^3
        if (x > x_length_/2) {
          heights_.at(x+z*x_length_) = prev_cliff_x3_rand_*(norm_x*norm_x*norm_x);
        } else {
          heights_.at(x+z*x_length_) = prev_water_x3_rand_*(norm_x*norm_x*norm_x);
        }
      }
    }
  }

  // Randomize Bottom Terrain
  for (int i = start; i < end; ++i) {
    int v = rand() % 4 + 1;
    switch(v) {
      case 1: x_water_position_++;
              break;
      case 2: x_water_position_--;
              break;
      case 3: z_water_position_++;
              break;
      case 4: z_water_position_--;
              break;
    }
    if (x_water_position_ < 0) {
      x_water_position_ = x_length_/2-2 * length_multiplier_;
      continue;
    } else if (x_water_position_ > x_length_/2-2 * length_multiplier_) {
      x_water_position_ = 0;
      continue;
    }
    if (z_water_position_ < 0) {
      z_water_position_ = z_length_-1;
      continue;
    } else if (z_water_position_ > z_length_-1) {
      z_water_position_ = 0;
      continue;
    }
    heights_.at(x_water_position_ + z_water_position_*x_length_) -= 0.100f;
  }

  // Randomize Top Terrain
  for (int i = start; i < end; ++i) {
    int v = rand() % 4 + 1;
    switch(v) {
      case 1: x_cliff_position_++;
              break;
      case 2: x_cliff_position_--;
              break;
      case 3: z_cliff_position_++;
              break;
      case 4: z_cliff_position_--;
              break;
    }
    if (x_cliff_position_ < x_length_/2+4 * length_multiplier_) {
      x_cliff_position_ = x_length_-1;
      continue;
    } else if (x_cliff_position_ > x_length_-1) {
      x_cliff_position_ = x_length_/2+4 * length_multiplier_;
      continue;
    }
    if (z_cliff_position_ < 0) {
      z_cliff_position_ = z_length_-1;
      continue;
    } else if (z_cliff_position_ > z_length_-1) {
      z_cliff_position_ = 0;
      continue;
    }
    heights_.at(x_cliff_position_ + z_cliff_position_*x_length_) += 0.100f;
  }

}

// Smooths the terrain at the connections
//   Spreads the load over 2 ticks
//   @param bool, whether or not this is the first call
//   @warn requires a last row member
//   @warn AverageVector modifies heights_ member
void Terrain::HelperMakeSmoothHeights(const bool is_first_call) {
  if (is_first_call) {
    // EXTEND FLOOR (REMOVES LONG DISTANCE ARTEFACTS)
    int x = 0; // last is constant
    for (int z = 0; z < z_length_; ++z) {
      heights_.at(x + z*x_length_) = -40.0f;
    }
    // for (int x = 1; x < 4; ++x) {
    //   for (int z = 0; z < z_length_; ++z) {
    //     heights_.at(x + z*x_length_) -= 20.0f;
    //   }
    // }

    // SMOOTH CONNECTIONS
    // Compare connection rows to eachother and smooth new one
    for (int x = 0; x < x_length_; ++x) {
      heights_.at(x+0*x_length_) = temp_last_row_heights_.at(x);
      // heights_.at(x+0*x_length_) = 0;
    }

    // SMOOTH WATER TERRAIN
    AverageVector(1, x_length_/2-4, heights_, temp_last_row_heights_);
    return;
  } 
  // SMOOTH CLIFF TERRAIN
  AverageVector((x_length_/2+5), x_length_-1, heights_, temp_last_row_heights_);
  AverageVector((x_length_/2+5), x_length_-1, heights_, temp_last_row_heights_);
  // AverageVector((x_length_/2+15), x_length_-1, heights_, temp_last_row_heights_);
  // AverageVector((x_length_/2+15), x_length_-1, heights_, temp_last_row_heights_);
  // AverageVector((x_length_/2+15), x_length_-1, heights_, temp_last_row_heights_);

}

// Averages the given member to smooth the terrain
//   Has a range for X but runs through the entire Z plane (for splitting water 
//   and cliff
//   @param start, the start of the heightmap in the X plane
//   @param end,   the end of the heightmap in the X plane
//   @param vec_t, a reference to a vector which contains heightmap values or vec3 and
//             will be modified. Infact any vector type with + and /= element operators
//             should work.
//   @param vec_other_t, a reference to a vector which contains @vec_t values from the
//                       previous tile
//   @warn @a vec_t member is modified
template<typename T>
void Terrain::AverageVector(const int start, const int end, std::vector<T> &vec_t, const std::vector<T> &vec_other_t) {
  int z = 0;
  int x = start;
  for (; x < end; ++x) {
    // Get all elements around center
    //   Orientation is from car start facing
    T &center = vec_t.at(x + z*x_length_);
    const T &left = vec_t.at(x+1 + z*x_length_);
    const T &right = vec_t.at(x-1 + z*x_length_);
    const T &bot = vec_other_t.at(x);
    const T &top = vec_t.at(x + (z+1)*x_length_);
    const T &top_left = vec_t.at(x+1 + (z+1)*x_length_);
    const T &top_right = vec_t.at(x-1 + (z+1)*x_length_);
    const T &bot_left = vec_other_t.at(x+1);
    const T &bot_right = vec_other_t.at(x-1);

    T average = (center+top+bot+left+right
        +top_left+top_right+bot_left+bot_right);
    average /= 9.0f;
    center = average;  //pass by reference
  }
  // new scope
  {
    x = end;
    // Get all elements around center
    //   Orientation is from car start facing
    T &center = vec_t.at(x + z*x_length_);
    const T &left = T();
    const T &right = vec_t.at(x-1 + z*x_length_);
    const T &bot = vec_other_t.at(x);
    const T &top = vec_t.at(x + (z+1)*x_length_);
    const T &top_left = T();
    const T &top_right = vec_t.at(x-1 + (z+1)*x_length_);
    const T &bot_left = T();
    const T &bot_right = vec_other_t.at(x-1);

    T average = (center+top+bot+left+right
        +top_left+top_right+bot_left+bot_right);
    average /= 9.0f;
    center = average;  //pass by reference
  }

  z = 1;
  for (; z < z_length_ - 1; ++z) {
    int x = start;
    for (; x < end; ++x) {
      // Get all elements around center
      //   Orientation is from car start facing
      T &center = vec_t.at(x + z*x_length_);
      const T &left = vec_t.at(x+1 + z*x_length_);
      const T &right = vec_t.at(x-1 + z*x_length_);
      const T &bot = vec_t.at(x + (z-1)*x_length_);
      const T &top = vec_t.at(x + (z+1)*x_length_);
      const T &top_left = vec_t.at(x+1 + (z+1)*x_length_);
      const T &top_right = vec_t.at(x-1 + (z+1)*x_length_);
      const T &bot_left = vec_t.at(x+1 + (z-1)*x_length_);
      const T &bot_right = vec_t.at(x-1 + (z-1)*x_length_);

      T average = (center+top+bot+left+right
          +top_left+top_right+bot_left+bot_right);
      average /= 9.0f;
      center = average;  //pass by reference
    }
    x = end;
    // Get all elements around center
    //   Orientation is from car start facing
    T &center = vec_t.at(x + z*x_length_);
    const T &left = T();
    const T &right = vec_t.at(x-1 + z*x_length_);
    const T &bot = vec_t.at(x + (z-1)*x_length_);
    const T &top = vec_t.at(x + (z+1)*x_length_);
    const T &top_left = T();
    const T &top_right = vec_t.at(x-1 + (z+1)*x_length_);
    const T &bot_left = T();
    const T &bot_right = vec_t.at(x-1 + (z-1)*x_length_);

    T average = (center+top+bot+left+right
        +top_left+top_right+bot_left+bot_right);
    average /= 9.0f;
    center = average;  //pass by reference
  }
  z = z_length_-1;
  for (x = start; x < end; ++x) {
    // Get all elements around center
    //   Orientation is from car start facing
    T &center = vec_t.at(x + z*x_length_);
    const T &left = vec_t.at(x+1 + z*x_length_);
    const T &right = vec_t.at(x-1 + z*x_length_);
    const T &bot = vec_t.at(x + (z-1)*x_length_);
    const T &bot_left = vec_t.at(x+1 + (z-1)*x_length_);
    const T &bot_right = vec_t.at(x-1 + (z-1)*x_length_);
    T average = (center+bot+left+right
        +bot_left+bot_right);
    average /= 6.0f;
    center = average;  //pass by reference
  }
  // New scope
  {
    x = end;
    // Get all elements around center
    //   Orientation is from car start facing
    T &center = vec_t.at(x + z*x_length_);
    const T &left = T();
    const T &right = vec_t.at(x-1 + z*x_length_);
    const T &bot = vec_t.at(x + (z-1)*x_length_);
    const T &top_left = T();
    const T &bot_left = T();
    const T &bot_right = vec_t.at(x-1 + (z-1)*x_length_);

    T average = (center+bot+left+right
        +top_left+bot_left+bot_right);
    average /= 7.0f;
    center = average;  //pass by reference
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
void Terrain::HelperMakeVertices(const RoadType road_type, const TileType tile_type,
    const float min_position, const float position_range) {
  // Store the connecting row to smooth
  std::vector<glm::vec3>::iterator z_smooth_begin = vertices.end()-1*x_length_;
  std::vector<glm::vec3>::iterator z_smooth_end = vertices.end()-0*x_length_;
  std::vector<glm::vec3> temp_last_row_vertices(z_smooth_begin, z_smooth_end);

  // Zero vertice vector
  // vertices.assign(x_length_ * z_length_, glm::vec3());

  int offset;
  // First, build the data for the vertex buffer
  for (int z = 0; z < z_length_; z++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (z*x_length_)+x;
      float xRatio = x / (float) (x_length_ - 1);

      // Build our heightmap from the top down, so that our triangles are 
      // counter-clockwise.
      float zRatio = (z / (float) (z_length_ - 1));

      float xPosition = min_position + (xRatio * position_range);
      float yPosition = heights_.at(offset);
      float zPosition = min_position + (zRatio * position_range);

      // Water or Terrain
      // switch(tile_type) {
      //   case kTerrain:
      //     yPosition = heights_.at(offset);
      //     break;
      //   case kRoad:
      //     assert(0 && "improper use of function");
      //     break;
      // }

      // Curve addition
      switch(road_type) {
        // case 0: straight road => do nothing
        // case 1: x^2 turnning road
        case kTurnLeft:
          {
            float zSquare = zPosition * zPosition; //x^2
            xPosition = zSquare/(position_range*5.5) + xPosition;
            break;
          }
        case kTurnRight:
          {
            float zSquare = zPosition * zPosition; //x^2
            xPosition = -zSquare/(position_range*5.5) + xPosition;
            break;
          }
      }

      vertices.at(offset) = glm::vec3(xPosition, yPosition, zPosition);
    }
  }
  // special point for finding pivot translation
  unsigned int pivot_x = 18 * length_multiplier_; // relative tile x position of pivot
  const glm::vec3 &pivot = vertices.at(pivot_x);
  // pivot.y = 10000.0f;
  // Rotate the point using glm function
  glm::vec3 rotated = glm::rotateY(pivot, rotation_);
  float translate_x = pivot.x - rotated.x;
  float translate_z = pivot.z - rotated.z;
  for (int y = 0; y < z_length_; y++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (y*x_length_)+x;

      glm::vec3 rotated = vertices.at(offset);
      rotated = glm::rotateY(rotated, rotation_);
      float xPosition = rotated.x + translate_x;
      float zPosition = rotated.z + translate_z;
      // xPosition = rotated.x + position_range/2;
      float yPosition = rotated.y;

      vertices.at(offset) = glm::vec3(xPosition + next_tile_start_.x, yPosition,
          zPosition + next_tile_start_.y);
    }
  }
  // Water or Terrain
  switch(tile_type) {
    case kTerrain:
      const glm::vec3 &pivot_end = vertices.at(pivot_x + (z_length_-1)*x_length_);
      // Set next z position
      next_tile_start_.y = pivot_end.z;

      // Calculate next_tile_start_.x position (next X tile position)
      float displacement_x = pivot_end.x - pivot.x;
      next_tile_start_.x += displacement_x;
      break;
  }
  switch(road_type) {
    case kTurnLeft:
      {
        // generate random number between 18.00 and 24.99
        float random = rand() % 700 / 100.0f + 18;
        rotation_ += random;
        // rotation_ += 18.0f;
        break;
      }
    case kTurnRight:
      {
        // generate random number between 18.00 and 24.99
        float random = rand() % 700 / 100.0f + 18;
        rotation_ -= random;
        break;
      }
  }

  // Make Left side infinite
  float const cos_rot = cos(DEG2RAD(prev_rotation_)); //optimization
  float const sin_rot = -sin(DEG2RAD(prev_rotation_)); //optimization
  prev_rotation_ = rotation_;
  for (int x = 0; x < 4; ++x) {
    for (int z = 0; z < z_length_; ++z) {
      float &vert_x = vertices.at((x_length_-x-1)+z*x_length_).x;
      vert_x += 40 * cos_rot;
      float &vert_z = vertices.at((x_length_-x-1)+z*x_length_).z;
      vert_z += 40 * sin_rot;
    }
  }
  // Make Right side infinite
  for (int x = 0; x < 4; ++x) {
    for (int z = 0; z < z_length_; ++z) {
      float &vert_x = vertices.at(x+z*x_length_).x;
      vert_x -= 40 * cos_rot;
      float &vert_z = vertices.at(x+z*x_length_).z;
      vert_z -= 40 * sin_rot;
    }
  }
  // Give left side structure - RELAX O(4N)
  for (int x = 0; x < 4; ++x) {
    // Try to remove first couple layers going too high
    int z = 0;
    // while (z <= z_smooth_max_) {
    //   float &vert_y = vertices.at((x_length_-x-1)+z*x_length_).y;
    //     vert_y = -20.0f;
    //   ++z;
    // }
    // Randomly decrease slope
    int r = rand() % 40 + 10;
    for (; z < z_length_; ++z) {
      float &vert_y = vertices.at((x_length_-x-1)+z*x_length_).y;
      vert_y -= r;
    }
  }
  // SMOOTH CONNECTIONS
  // Compare connection rows to eachother and smooth new one
  std::vector<glm::vec3> translate_column_by;
  for (int x = 0; x < x_length_; ++x) {
    glm::vec3 dis_between_smooth = vertices.at(x+(z_smooth_max_)*x_length_) - temp_last_row_vertices.at(x+0*x_length_);
    dis_between_smooth.x /= z_smooth_max_;
    dis_between_smooth.z /= z_smooth_max_;
    glm::vec3 new_column_size = dis_between_smooth;
    glm::vec3 translate_by = new_column_size;
    translate_column_by.push_back(translate_by);
    // printf("trans = %f,%f\n",translate_by.x,translate_by.z);
  }
  for (int z = 0; z < z_smooth_max_; ++z) {
    for (int x = 0; x < x_length_; ++x) {
      float &vert_x = vertices.at(x+z*x_length_).x;
      vert_x = temp_last_row_vertices.at(x).x + z * translate_column_by.at(x).x;

      float &vert_z = vertices.at(x+z*x_length_).z;
      vert_z = temp_last_row_vertices.at(x).z + z * translate_column_by.at(x).z;
    }
  }
  // Ensure heights are connected
  for (int x = 0; x < x_length_; ++x) {
    float &vert_y = vertices.at(x+0*x_length_).y;
    vert_y = temp_last_row_vertices.at(x).y;
  }

  // Smooth left side structure LOOKS BAD
  // AverageVector(x_length_-4,x_length_-1,vertices, temp_last_row_vertices);

  // Fix UV coordinates LOOKS WORSE (LOGIC ERROR)
  // HelperFixUV(translate_column_by, position_range);

}

// Fixes the UV caused by a changing z_smooth_max_
//   @warn changes UV for terrain
void Terrain::HelperFixUV(const std::vector<glm::vec3> &translated_by, const float position_range) {
  // FIX TERRAINUV COORDINATES
  for (int z = 0; z < z_smooth_max_; z++) {
    for (int x = 0; x < x_length_; x++) {
      int offset = (z*x_length_)+x;
      float xRatio = x / (float) (x_length_ - 1);
      float zRatio = (z / (float) (z_length_ - 1));
      // zRatio /= (x_length_/20)*(40+x_length_/20);
      zRatio *= 1-(translated_by.at(z).z-position_range/z_length_);
      xRatio *= 1-(translated_by.at(x).x-position_range/x_length_);

      texture_coordinates_uv.at(offset) = glm::vec2(
          xRatio*float(z_length_)*0.10f,
          zRatio*float(z_length_)*0.10f / length_multiplier_);
    }
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
      float yRatio = (y / (float) (z_length_ - 1));
      // Textures need to be less frequent at infinity spots
      if (x > x_length_ - 5) {
        yRatio /= (x_length_/20)*(40+x_length_/20);
        xRatio /= (x_length_/20)*(40+x_length_/20);
      } else if (x < 4) {
        yRatio /= (x_length_/20)*(40+x_length_/20);
        xRatio /= (x_length_/20)*(40+x_length_/20);
      }

      texture_coordinates_uv.at(offset) = glm::vec2(
          xRatio*float(z_length_)*0.10f,
          yRatio*float(z_length_)*0.10f / length_multiplier_);
    }
  }
}

// Generates the normals by doing a cross product of neighbouring vertices
// @warn  No changes can be made to normals_ member until the Road Helpers complete
void Terrain::HelperMakeNormals() {
  // normals.assign(x_length_*z_length_, glm::vec3());
  // Reset normals vector but save last X row to start
  for (int i = 0, x = normals.size()-x_length_-1; 
      x < normals.size(); ++i, ++x) {
    normals.at(i) = normals.at(x);
  }
  std::fill(normals.begin()+1,normals.end(), glm::vec3()); //fill rest with zero
  // for ( unsigned int i = z_smooth_max_*x_length_; i < indices.size()-2; i += 3 )  {
  for ( unsigned int i = 0; i < indices.size()-2; i += 3 )  {
    glm::vec3 v0 = vertices[ indices[i + 0] ];
    glm::vec3 v1 = vertices[ indices[i + 1] ];
    glm::vec3 v2 = vertices[ indices[i + 2] ];

    glm::vec3 normal = glm::normalize( glm::cross( v1 - v0, v2 - v0 ) );
    // printf("norm = (%f,%f,%f)\n",normal.x,normal.y,normal.z);

    // Remove NaN normals
    // if (normal.x != normal.x) {
    // printf("Overlapping vertices being crossed\n");
    // } else {
    normals[ indices[i + 0] ] += normal;
    normals[ indices[i + 1] ] += normal;
    normals[ indices[i + 2] ] += normal;
    // }
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
  for (unsigned int x = 15 * length_multiplier_; x < 19 * length_multiplier_; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z){
      vertices_road_.push_back(vertices.at(x + z*x_length_));
      // Lift road a bit above terrain to make it visible
      vertices_road_.back().y += 0.01f;
      // vertices_road_.back().y += 0.01f + 0.02*rotation_;
    }
  }

  // Store water vertices
  std::vector<glm::vec3> water_side;
  water_side.reserve((x_length_ - (15 * length_multiplier_))*z_length_);
  for (unsigned int x = 0; x < 15 * length_multiplier_; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z) {
      water_side.push_back(vertices.at(x + z*x_length_)); // other side vertices
    }
  }
  colisn_lst_water_.push_back(water_side);

  // Store cliff vertices (only 1 row)
  std::vector<glm::vec3> cliff_side;
  cliff_side.reserve(1*z_length_);
  // NOTE the +1 in below loop is a tweak for 96x96 terrain
  for (unsigned int x = 19 * length_multiplier_+1; x < 20 * length_multiplier_+1; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z) {
      cliff_side.push_back(vertices.at(x + z*x_length_)); // other side vertices
    }
  }
  colisn_lst_cliff_.push_back(cliff_side);
}

// Rip the road parts of the terrain normals vector using calulcated magic numbers and store
// these in the normals_road_ vector
// @warn  requires a preceeding call to HelperMakeNormals otherwise undefined behaviour
// @warn  for optimzation this should only be called once because road normals don't change
void Terrain::HelperMakeRoadNormals() {
  // normals_road_.clear();
  // for (unsigned int x = 15 * length_multiplier_; x < 19 * length_multiplier_; ++x) {
  //   for (unsigned int z = 0; z < z_length_; ++z){
  //     normals_road_.push_back(normals.at(x + z*x_length_));
  //   }
  // }
  normals_road_.assign((4*length_multiplier_)*z_length_, glm::vec3(0,1,0));
}

// Rip the road parts of the terrain indice and UV vectors using calulcated magic numbers and 
// store these in the indice and UV vectors respectfully
// @warn  requires a preceeding call to HelperMakeIndicesAndUV otherwise undefined behaviour
// @warn  for optimzation this should only be called once because road indices and UV don't change
void Terrain::HelperMakeRoadIndicesAndUV() {
  // Create Index Data
  // 31 quads in row, 2 triangles in quad, 3 rows, 3 vertices per triangle
  //   Above based on 32 x_length_ & z_length_
  // indices_road_.assign(indices.begin(), indices.begin()+31*2*3*3);
  indices_road_.assign(indices.begin(), indices.begin()+(x_length_-1)*2*((18-15)*length_multiplier_)*3);

  // CONSTRUCT UV COORDINATES
  std::vector<glm::vec2> temp_uv;
  temp_uv.assign(x_length_*z_length_, glm::vec2());
  int offset;
  // First, build the data for the vertex buffer
  for (int y = 0; y < z_length_; y++) {
    for (int x = 0; x < x_length_; x++) {
      offset = (y*x_length_)+x;
      float xRatio = x / (float) (x_length_ - 1);
      float yRatio = (y / (float) (z_length_ - 1));

      temp_uv.at(offset) = glm::vec2(
          xRatio*float(z_length_)*0.10f * 3.2f / length_multiplier_,
          yRatio*float(z_length_)*0.10f * 1.0f / length_multiplier_);
    }
  }
  // Create UV Data
  texture_coordinates_uv_road_.clear();
  for (unsigned int x = 0; x < 5*length_multiplier_; ++x) {
    for (unsigned int z = 0; z < z_length_; ++z){
      // The multiplications below change stretch of the texture (ie repeats)
      texture_coordinates_uv_road_.push_back(glm::vec2(
            temp_uv.at(x + z*x_length_).x,
            temp_uv.at(x + z*x_length_).y));
    }
  }

}

// Generates a collision coordinate mapping
//   Finds all edge vertices of road in order then pairs them with the closest vertices
//   on the opposite side of the road
// @warn  requires a preceeding call to HelperMakeRoadVertices otherwise undefined behaviour
// @warn  this is O(n^2) with n = 36  TODO improve complexity
void Terrain::HelperMakeRoadCollisionMap() {
  unsigned int x_new_row_size = 18 * length_multiplier_ - 15 * length_multiplier_;
  std::vector<glm::vec3> left_side, right_side;
  left_side.reserve(z_length_);
  right_side.reserve(z_length_);
  // Make both sides
  for (unsigned int z = 0; z < z_length_; ++z){
    const glm::vec3 &left = vertices_road_.at(0 + z);
    const glm::vec3 &right = vertices_road_.at(z + z_length_ * (x_new_row_size));
    left_side.push_back(left); // left? side vertices
    right_side.push_back(right); // other side vertices
  }

  colisn_vec tile_map;
  tile_map.reserve(z_length_);
  std::pair<glm::vec3,glm::vec3> min_max_x_pair;
  // Pair left side to it's closest point on opposite side
  for (unsigned int z = 0; z < z_length_; ++z) {
    const glm::vec3 &left = left_side.at(z);
    min_max_x_pair.first = left;
    float smallest_diff = glm::distance(left_side.at(z), right_side.front());
    glm::vec3 closest_point = right_side.front();
    for (unsigned int x = 1; x < z_length_; ++x) {
      float curr_diff = glm::distance(left_side.at(z), right_side.at(x));
      if (curr_diff < smallest_diff) {
        smallest_diff = curr_diff;
        closest_point = right_side.at(x);
      }
    }
    min_max_x_pair.second = closest_point;

    tile_map.push_back(min_max_x_pair); // doesnt insert when duplicate
  }

  collision_queue_hash_.push_back(tile_map);
}

// Creates a new vertex array object and loads in data into a vertex attribute buffer
//   The parameters are self explanatory.
//   @return vao_handle, the vao handle
unsigned int Terrain::CreateVao(const GLuint program_id, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
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

#ifndef ASSIGN3_TERRAIN_H_
#define ASSIGN3_TERRAIN_H_

#include <vector>
#include <string>
#include <cassert>
#include <queue>
#include <ctime>
#include <algorithm>

#include "model_data.h"
#include "model.h"
#include "camera.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "lib/shader/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "lib/circular_vector/circular_vector.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Terrain {
  public:
    // These are used for collisions and it's helper functions
    typedef std::pair<glm::vec3, glm::vec3> boundary_pair;
    typedef std::vector<boundary_pair> colisn_vec;

    // Construct with width and height specified
    Terrain(const GLuint &program_id, const int &width = 32, const int &height = 32);
    
    // Accessor for the VAO
    // TODO comment
    inline circular_vector<unsigned int> terrain_vao_handle() const;
    // Accessor for the program id (shader)
    inline GLuint terrain_program_id() const;
    // TODO comment
    inline circular_vector<unsigned int> road_vao_handle() const;
    // TODO comment
    inline GLuint road_texture() const;
    // Accessor for the loaded texture
    //   @warn requires a texture to be loaded with LoadTexture()
    inline GLuint texture() const;
    // Accessor for the width (Amount of Grid boxes width-wise)
    inline int width() const;
    // Accessor for the height (Amount of Grid boxes height-wise)
    inline int height() const;
    // Accessor for the amount of indices
    //   Used in render to efficiently draw triangles
    inline int indice_count() const;
    // Accessor for the amount of indices
    //   Used in render to efficiently draw triangles
    inline int road_indice_count() const;
    // Accessor for the collision checking data structure
    //   See the collision_queue_hash_ member var (or this func implementation) for details
    inline std::queue<colisn_vec> collision_queue_hash() const;
    // Pops the first collision map
    //   To be used after car has passed road tile
    //   TODO remove and replace in circular buffer instead
    //   @warn this is a test function (shouldn't be inline either)
    inline void col_pop();
    // Generates next tile and removes first one
    //   Uses the circular_vector data structure to do this in O(1)
    //   TODO merge col_pop or something
    void ProceedTiles();

  private:
    // CONSTANTS
    enum TileType {
      kTerrain = 0,
      kRoad = 1,
    };
    enum RoadType {
      kStraight = 0,
      kTurnLeft = 1,
      kTurnRight = 2,
    };
    // Width of the heightmap
    int x_length_;
    // Height of the heightmap
    int z_length_;

    // RENDER DATA
    // The amount of indices, used to render terrain efficiently
    int indice_count_;
    // The amount of indices in a straight road piece, used to render efficiently
    int road_indice_count_;
    // The shader to use to render heightmap
    //   Road uses the same shader
    GLuint terrain_program_id_;
    // The texture to be used to Wrap Terrain
    GLuint texture_;
    // The texture to be used for the road
    GLuint road_texture_;
    // The VAO handle for the terrain
    circular_vector<unsigned int> terrain_vao_handle_;
    // The VAO handle for the road
    circular_vector<unsigned int> road_vao_handle_;
    // A circular_vector representing each road tile for collision checking
    //   The first (0th) index is the current tile the car is on (or not - check index 1)
    //   Each key in the map represents a Z scanline
    //   Each pair of values of the key represents it's min X and max X
    //     i.e. it's bounding box of the road
    //     pair.first = min_x, pair.second = max_x
    std::queue<colisn_vec> collision_queue_hash_;
    // A queue of rotations corresponding to each collision tile
    //   Used to reset car to current orientation on road after crash
    std::queue<float> rotation_queue_;

    // GENERATE TERRAIN VARS
    // Vertices to be generated for next terrain (or water) tile
    std::vector<glm::vec3> vertices;
    // Normals to be generated for the next terrain (or water) tile
    std::vector<glm::vec3> normals;
    // The texture coordinates generated for all the terrain and water tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.
    std::vector<glm::vec2> texture_coordinates_uv;
    // The indices generated for all the terrain and water tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.
    std::vector<int> indices;
    // This vector is used to build heights and smooths previous tile connections
    std::vector<float> heights_; 

    // The current X,Z displacement from zero
    //   Used for joining tiles
    glm::vec2 next_tile_start_;
    // The previous maximum x vertice
    //   Used for updating next_tile_start_.x
    float prev_max_x_;

    // GENERATE ROAD VARS
    // Vertices to be generated for the next road tile
    std::vector<glm::vec3> vertices_road_;
    // Normals to be generated for all the road tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.and the normals always point upwards (road is flat)
    std::vector<glm::vec3> normals_road_;
    // The texture coordinates generated for all the road tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.
    std::vector<glm::vec2> texture_coordinates_uv_road_;
    // The indices generated for all the road tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.
    std::vector<int> indices_road_;
    // Current road tile end rotation
    //   The rotation of the entire next tile from positive z
    //   Positive degrees rotate leftwards (anti cw from spidermans facing)
    float rotation_;
    // The amount of (tile relative) Z rows from the back to smooth
    //   Is needed to connect rotated rows
    unsigned int z_smooth_max_;

    // Generates a random terrain piece and pushes it back into circular_vector VAO buffer
    void RandomizeGeneration();
    // Generate Terrain tile piece with road
    //   Mutates the input members, (e.g. vertices, indicies etc.) and then
    //   calls CreateVAO and pushes the result back to the terrain_vao_handle_
    //   @param The tile type to generate e.g. kStraight, kTurnLeft etc.
    //   @warn creates and pushes back a road VAO based on the terrain middle section
    //   @warn pushes next road collision map into member queue
    void GenerateTerrain(RoadType road_type);

    // TERRAIN GENERATION HELPERS
    // Model the heights using an X^3 mathematical functions, then randomize heights
    // for all vertices in heightmap
    //   @warn pretty expensive operation 10000*2 loops
    void HelperMakeHeights();
    // Overloaded function to generate a square height map on the X/Z plane. Different
    // road_type parameters can be added to curve the Z coordinates and hence make turning
    // pieces.
    // @param  road_type       An enum representing the mathematical model to be applied to Z
    // @param  tile_type       An enum representing whether the tile is water or terrain
    // @param  min_position    The relative start position of the heightmap over X/Z
    // @param  position_range  The spread of the heightmap over X/Z 
    // @warn  No changes can be made to vertices_ member until the Road Helpers complete
    void HelperMakeVertices(RoadType road_type = kStraight, TileType tile_type = kTerrain,
        float min_position = 0.0f, float position_range = 20.0f);
    // Generates the indices and UV texture coordinates to be used by the tile
    // @note  These don't change for the same x_length_ * z_length_ height maps
    // @warn  No changes can be made to indices or UV member until the Road Helpers complete
    void HelperMakeIndicesAndUV();
    // Generates the normals by doing a cross product of neighbouring vertices
    // @warn  No changes can be made to normals_ member until the Road Helpers complete
    void HelperMakeNormals();

    // ROAD GENERATION HELPERS
    // Rip the road parts of the terrain vertice vector using calulcated magic numbers and store
    // these in the vertices_road_ vector
    // @warn  requires a preceeding call to HelperMakeVertices otherwise undefined behaviour
    void HelperMakeRoadVertices();
    // Fixes the UV caused by a changing z_smooth_max_
    //   @warn changes UV for both terrain and road
    void HelperFixUV();
    // Rip the road parts of the terrain normals vector using calulcated magic numbers and store
    // these in the normals_road_ vector
    // @warn  requires a preceeding call to HelperMakeNormals otherwise undefined behaviour
    // @warn  for optimzation this should only be called once because road normals don't change
    void HelperMakeRoadNormals();
    // Rip the road parts of the terrain indice and UV vectors using calulcated magic numbers and 
    // store these in the indice and UV vectors respectfully
    // @warn  requires a preceeding call to HelperMakeIndicesAndUV otherwise undefined behaviour
    // @warn  for optimzation this should only be called once because road indices and UV don't change
    void HelperMakeRoadIndicesAndUV();
    // Generates a collision coordinate mapping
    //   Using the vertices_road_ vector, maps all rounded Z coordinates to their corresponding min X
    //   and Max X coordinates. Then pushes the map into the member queue ready for collision checking.
    // @warn  requires a preceeding call to HelperMakeRoadVertices otherwise undefined behaviour
    void HelperMakeRoadCollisionMap();

    // OPENGL RENDERING FUNCTIONS
    // Creates a new vertex array object and loads in data into a vertex attribute buffer
    //   The parameters are self explanatory.
    //   @return vao_handle, the vao handle
    unsigned int CreateVao(const GLuint &program_id, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
        const std::vector<glm::vec2> &texture_coordinates_uv, const std::vector<int> &indices);
    // Creates a new vertex array object and loads in data into a vertex attribute buffer
    //   @param  tile_type  An enum representing the proper members to use
    //   @return vao_handle, the vao handle
    unsigned int CreateVao(TileType tile_type);
    // Creates a texture pointer from file
    //   @return  GLuint  The int pointing to the opengl texture data
    GLuint LoadTexture(const std::string &filename);

    // Verbose Debugging mode
    bool is_debugging_;
};

// Accessor for the VAO
// TODO comment
inline circular_vector<unsigned int> Terrain::terrain_vao_handle() const {
  return terrain_vao_handle_;
}
// Accessor for the program id (shader)
inline GLuint Terrain::terrain_program_id() const {
  return terrain_program_id_;
}
// TODO comment
inline circular_vector<unsigned int> Terrain::road_vao_handle() const {
  return road_vao_handle_;
}
// TODO comment
inline GLuint Terrain::road_texture() const {
  return road_texture_;
}
// Accessor for the loaded texture
//   @warn requires a texture to be loaded with LoadTexture()
inline GLuint Terrain::texture() const {
  return texture_;
}
// Accessor for the width (Amount of Grid boxes width-wise)
inline int Terrain::width() const {
  return x_length_;
}
// Accessor for the height (Amount of Grid boxes height-wise)
inline int Terrain::height() const {
  return z_length_;
}
// Accessor for the amount of indices
//   Used in render to efficiently draw triangles
inline int Terrain::indice_count() const {
  return indice_count_;
}
// Accessor for the amount of indices
//   Used in render to efficiently draw triangles
inline int Terrain::road_indice_count() const {
  return road_indice_count_;
}
// Accessor for the collision checking data structure
// A queue representing each road tile for collision checking
//   Each key in the map represents a Z scanline
//   Each pair of values of the key represents it's min X and max X
//     i.e. it's bounding box of the road
//     pair.first = min_x, pair.second = max_x
inline std::queue<Terrain::colisn_vec> Terrain::collision_queue_hash() const {
  return collision_queue_hash_;
}
// Pops the first collision map 
//   To be used after car has passed road tile
//   TODO remove and replace in circular buffer instead
//   @warn this is a test function (shouldn't be inline either)
inline void Terrain::col_pop() {
  collision_queue_hash_.pop();
}

#endif

#ifndef ASSIGN3_TERRAIN_H_
#define ASSIGN3_TERRAIN_H_

#include <vector>
#include <string>
#include <cassert>
#include <queue>
#include <ctime>
#include <algorithm>
#include <list>

#include "model_data.h"
#include "model.h"
#include "camera.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "shaders/shaders.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "lib/circular_vector/circular_vector.h"
// Check better performance container
// #include <deque>
// #define circular_vector std::deque

class Terrain {
  public:
    // These are used for collisions and it's helper functions
    typedef std::pair<glm::vec3, glm::vec3> boundary_pair;
    typedef std::vector<boundary_pair> colisn_vec;
    typedef std::vector<glm::vec3> anim_vec;
    typedef circular_vector<anim_vec> anim_container;
    // Constants
    enum RoadType {
      kStraight = 0,
      kTurnLeft = 1,
      kTurnRight = 2,
    };

    // TODO remove from public
    GLuint cliff_nrm_texture_;

    // Construct with width and height specified
    Terrain(const Shader &shader, const int width = 96, const int height = 96);

    // Accessor for the program id (shader)
    inline const Shader shader() const;
    // A container filled with tiled terrain VAOs in proceeding order
    inline const circular_vector<GLuint> * terrain_vao_handle() const;
    // A container filled with tiled road VAOs in proceeding order
    inline const circular_vector<GLuint> * road_vao_handle() const;
    // The GL generated road texture used for binding
    inline GLuint road_texture() const;

    inline GLuint cliff_bump() const;

    inline GLuint road_bump() const;
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
    //   See the colisn_boundary_pairs_ member var (or this func implementation) for details
    inline const circular_vector<colisn_vec> * colisn_boundary_pairs() const;
    // Accessor for the first pair for spawning car
    inline glm::vec3 colisn_boundary_pair_first() const;
    // Accessor for the last pair for spawning car
    inline glm::vec3 colisn_boundary_pair_last() const;
    // Accessor for the water collision checking data structure
    //   See this func implementation for details
    inline const circular_vector<std::vector<glm::vec3> > * colisn_lst_water() const;
    // Accessor for the cliff collision checking data structure
    //   See this func implementation for details
    inline const circular_vector<std::vector<glm::vec3> > * colisn_lst_cliff() const;
    // Accessor for the turn type for tile at index
    //   Used for road sign type spawn decision
    inline const circular_vector<RoadType> * tile_turn() const;
    // Pops the first collision map
    //   To be used after car has passed road tile
    void colisn_pop();
    // Generates next tile and removes first one
    //   Uses the circular_vector data structure to do this in O(1)
    //   Sets up for generating next terrain tile over several ticks
    void ProceedTiles();
    // Generates the next part of tile for spreading over multiple ticks
    void GenerationTick();

  private:
    // CONSTANTS
    enum TileType {
      kTerrain = 0,
      kRoad = 1,
    };
    // The amount of ticks to spread height generation over
    const signed char kHeightGenerationTicks = 20;
    // The amount of ticks to spread VAO creation over
    const signed char kVaoGenerationTicks = 5;
    // Width of the heightmap
    const unsigned char x_length_;
    // Height of the heightmap
    const unsigned char z_length_;
    // The multiplier for all magic numbers
    //   @warn this requires a square heightmap
    //   @warn dimensions should be multiples of 32
    const char length_multiplier_;
    // The maximum number of randomizing height generation iterations
    const int kRandomIterations;

    // RENDER DATA
    // The amount of ticks generated so far
    //   Used to spread iterations over multiple ticks
    //   Spread over $kGenerationTicks ticks
    signed char generated_ticks_;
    // The x and y positions of the height randomization for the (left) cliff part
    int x_cliff_position_;
    int z_cliff_position_;
    // The x and y positions of the height randomization for the (right) water part
    int x_water_position_;
    int z_water_position_;
    // The previous random value used to calculate next turn type
    //   Next turn rand is generated in proceedTiles
    char prev_rand_;
    // The previous random value used to generate the cliff and water (X^3 i.e. cubic) base heights
    //   Used to ensure there are no sudden peaks and for extra feel
    char prev_cliff_x3_rand_;
    char prev_water_x3_rand_;
    // The previous random value used to generate next spacing of tile
    float prev_spacing_rand_;
    // The shader to use to render heightmap
    //   Road uses the same shader
    const Shader shader_;
    // The texture to be used to Wrap Terrain
    GLuint texture_;
    // The bumpmap texture for the cliff
    GLuint cliff_bump_;
    // The bumpmap texture for the road
    GLuint road_bump_;
    // The texture to be used for the road
    GLuint road_texture_;
    // The VAO handle for the terrain and roads
    circular_vector<GLuint> terrain_vao_handle_;
    circular_vector<GLuint> road_vao_handle_;
    // The terrain and road VBOs assosicated with each VAO for deleting
    //   Each pair represents Vertices and Normals
    // @note  UV and Indices never change hence dont require delete
    circular_vector<std::pair<GLuint, GLuint> > terrain_vbo_handle_;
    circular_vector<std::pair<GLuint, GLuint> > road_vbo_handle_;
    // The road and terrain UV and Indice VBOs
    //   These dont change throughout life of terrain
    //   These are used in CreateVAO to optimize
    const std::pair<GLuint, GLuint> terrain_vbo_uv_indices_;
    const std::pair<GLuint, GLuint> road_vbo_uv_indices_;
    // The indices generated for all the tiles
    //   These should only be generated once as x_lengths and z_lengths are the
    //   same between tiles.
    const std::vector<int> indices_;
    const std::vector<int> indices_road_;
    // The amount of indices, used to render terrain efficiently
    const unsigned int indice_count_;
    const unsigned int road_indice_count_;
    // A circular_vector representing each road tile for collision checking
    //   The first (0th) index is the current tile the car is on (or not - check index 1)
    //     pair.first = min_x, pair.second = max_x
    circular_vector<colisn_vec> colisn_boundary_pairs_;
    // The collisions for the right (water) side
    circular_vector<std::vector<glm::vec3> > colisn_lst_water_;
    // The collisions for the left (cliff) side
    circular_vector<std::vector<glm::vec3> > colisn_lst_cliff_;

    // Road Sign Vars
    circular_vector<RoadType> tile_turn_;

    // GENERATE TERRAIN VARS
    // Vertices to be generated for next terrain (or water) tile
    std::vector<glm::vec3> vertices_;
    // Normals to be generated for the next terrain (or water) tile
    std::vector<glm::vec3> normals_;
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
    // Current road tile end rotation
    //   The rotation of the entire next tile from positive z
    //   Positive degrees rotate leftwards (anti cw from spidermans facing)
    float rotation_;
    // The above used for UV stretch correction;
    float prev_rotation_;
    // The amount of (tile relative) Z rows from the back to smooth
    //   Is needed to connect rotated rows
    unsigned int z_smooth_max_;
    // The last row used for smoothing
    std::vector<float> temp_last_row_heights_;

    // Generates a random terrain piece and pushes it back into circular_vector VAO buffer
    //   Starting terrain is generated all at once but flowing terrain generation is spread
    //   over $kGenerationTicks ticks
    //   @param Whether or not the terrain is the starting terrain
    void RandomizeGeneration(const bool is_start = false);
    // Generate Terrain tile piece with road
    //   Mutates the input members, (e.g. vertices, indicies etc.) and then
    //   calls CreateVAO and pushes the result back to the terrain_vao_handle_
    //   @param The tile type to generate e.g. kStraight, kTurnLeft etc.
    //   @warn creates and pushes back a road VAO based on the terrain middle section
    //   @warn pushes next road collision map into member queue
    void GenerateStartingTerrain(RoadType road_type);
    // Generate Terrain tile piece with road
    //   Mutates the input members, (e.g. vertices, indicies etc.) and then
    //   calls CreateVAO and pushes the result back to the terrain_vao_handle_
    //   The members are mutated over $kGenerationTicks to spread load
    //   @param The tile type to generate e.g. kStraight, kTurnLeft etc.
    //   @warn creates and pushes back a road VAO based on the terrain middle section
    //   @warn pushes next road collision map into member queue
    void GenerateTerrain(RoadType road_type);

    // INITIALIZATION HELPERS
    // Generates the indices and UV texture coordinates to be used by the tile
    // @param  The type of tile, road will automatically call HelperMakeRoadIndicesAndUV
    // @param  bool, if true populate the indices_ member only and dont make VBO
    // @note  These don't change for the same x_length_ * z_length_ height maps
    // @return  A pair of VBO handles (for use in VBO deletion)
    // @return  Dummy pair if just populating member
    std::pair<GLuint, GLuint> InitializeIndicesAndUV(const TileType tile_type) const;
    // Rip the road parts of the input terrain indice and UV vectors using calulcated magic numbers and
    // store these in the indice and UV vectors respectfully
    // @param  indices,                the indices vector from the terrain call
    // @param  texture_coordinates_uv, the uv vector from the terrain call
    // @note  Normal usage is for this to be called from inside HelperMakeIndicesAndUV(kRoad)
    // @warn  for optimzation this should only be called once because road indices and UV don't change
    // @warn  modifies the inputs!
    void InitializeRoadIndicesAndUV(std::vector<int> &indices, std::vector<glm::vec2> &texture_coordinates_uv) const;
    // Generates the indices to be used by the tile type
    // @param  The type of tile, Terrain or Road
    // @note  These don't change for the same x_length_ * z_length_ height maps
    // @return  A vector of indices for generating vertices
    std::vector<int> InitializeIndices(const TileType tile_type) const;

    // TERRAIN GENERATION HELPERS
    // Model the heights using an X^3 mathematical functions, then randomize heights
    // for all vertices in heightmap
    //   @param  start  Index to start looping from
    //   @param  end    Index to finish the loop
    //   @warn pretty expensive operation 10000*2 loops
    //   @warn spread over a couple of loops
    void HelperMakeHeights(int start, const int end);
    // Smooths the terrain at the connections
    //   Spreads the load over 2 ticks
    //   @param bool, whether or not this is the first call
    //   @warn requires a last row member
    //   @warn AverageHeights modifies heights_ member
    void HelperMakeSmoothHeights(const bool is_first_call);
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
    void AverageVector(const int start, const int end, std::vector<T> &vec_t, const std::vector<T> &vec_other_t);
    // Overloaded function to generate a square height map on the X/Z plane. Different
    // road_type parameters can be added to curve the Z coordinates and hence make turning
    // pieces.
    // @param  road_type       An enum representing the mathematical model to be applied to Z
    // @param  tile_type       An enum representing whether the tile is water or terrain
    // @param  min_position    The relative start position of the heightmap over X/Z
    // @param  position_range  The spread of the heightmap over X/Z 
    // @warn  No changes can be made to vertices_ member until the Road Helpers complete
    void HelperMakeVertices(const RoadType road_type = kStraight, const TileType tile_type = kTerrain,
        const float min_position = 0.0f, const float position_range = 20.0f);
    // Generates the normals by doing a cross product of neighbouring vertices
    // @warn  No changes can be made to normals_ member until the Road Helpers complete
    void HelperMakeNormals();

    // ROAD GENERATION HELPERS
    // Rip the road parts of the terrain vertice vector using calulcated magic numbers and store
    // these in the vertices_road_ vector
    // @return  A pair of VBO handles (for use in VBO deletion)
    void HelperMakeRoadVertices();
    // Rip the road parts of the terrain normals vector using calulcated magic numbers and store
    // these in the normals_road_ vector
    // @warn  requires a preceeding call to HelperMakeNormals otherwise undefined behaviour
    // @warn  for optimzation this should only be called once because road normals don't change
    void HelperMakeRoadNormals();
    // Generates a collision coordinate mapping
    //   Finds all edge vertices of road in order then pairs them with the closest vertices
    //   on the opposite side of the road
    // @warn  requires a preceeding call to HelperMakeRoadVertices otherwise undefined behaviour
    // @warn  this is O(n^2) with n = 36  TODO improve complexity
    void HelperMakeRoadCollisionMap();

    // OPENGL RENDERING FUNCTIONS
    // Creates a new vertex array object and loads in data into a vertex attribute buffer
    //   @param vertices, the vertices of the heightmap
    //   @param normals, the normals of the heightmap
    //   @param uv_indices, a pair of VBOs: first = indices, second = UV
    //   @param vbo_handle, the next vertex/normal pair for deletion reference
    //   @return vao_handle, the vao handle
    //   @warn modifies vbo_handle purposefully
    GLuint CreateVao(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
        const std::pair<GLuint, GLuint> &uv_indices, circular_vector<std::pair<GLuint, GLuint> > &vbo_handle);
    // Creates a new vertex array object and loads in data into a vertex attribute buffer
    //   @param  tile_type  An enum representing the proper members to use
    //   @return vao_handle, the vao handle
    GLuint CreateVao(TileType tile_type);
    // Creates a texture pointer from file
    //   @return  GLuint  The int pointing to the opengl texture data
    GLuint LoadTexture(const std::string &filename) const;

    // Verbose Debugging mode
    bool is_debugging_;
};

inline GLuint Terrain::cliff_bump() const {
  return cliff_bump_;
}

inline GLuint Terrain::road_bump() const {
  return road_bump_;
}
// Accessor for the VAO
// TODO comment
inline const circular_vector<GLuint> * Terrain::terrain_vao_handle() const {
  return &terrain_vao_handle_;
}
// Accessor for the Shader object
inline const Shader Terrain::shader() const {
  return shader_;
}
// TODO comment
inline const circular_vector<unsigned int> * Terrain::road_vao_handle() const {
  return &road_vao_handle_;
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
//     i.e. it's bounding box of the road
//     pair.first = min_x, pair.second = max_x
inline const circular_vector<Terrain::colisn_vec> * Terrain::colisn_boundary_pairs() const {
  return &colisn_boundary_pairs_;
}
// Accessor for the first pair for spawning car
glm::vec3 Terrain::colisn_boundary_pair_first() const {
  boundary_pair p = colisn_boundary_pairs_.back().back();
  // Find midpoint
  glm::vec3 road_midpoint = (p.first+p.second);
  road_midpoint /= 2;
  road_midpoint.y = p.first.y;
  // Find lane midpoints
  glm::vec3 left_lane_midpoint = road_midpoint + p.second;
  left_lane_midpoint /= 2;

  return left_lane_midpoint;
}
// Accessor for the last pair for spawning car
glm::vec3 Terrain::colisn_boundary_pair_last() const {
  boundary_pair p = colisn_boundary_pairs_.back().back();
  // Find midpoint
  glm::vec3 road_midpoint = (p.first+p.second);
  road_midpoint /= 2;
  road_midpoint.y = p.first.y;
  // Find lane midpoints
  glm::vec3 right_lane_midpoint = road_midpoint + p.first;
  right_lane_midpoint /= 2;

  return right_lane_midpoint;
}
// Accessor for the water collision checking data structure
//   Holds all vertices right (water) side of road for crashing animation
inline const circular_vector<std::vector<glm::vec3> > * Terrain::colisn_lst_water() const {
  return &colisn_lst_water_;
}
// Accessor for the water collision checking data structure
//   Holds a line of vertices left (cliff) side of road for crashing animation
inline const circular_vector<std::vector<glm::vec3> > * Terrain::colisn_lst_cliff() const {
  return &colisn_lst_cliff_;
}
// Accessor for the turn type for tile at index
//   Used for road sign type spawn decision
inline const circular_vector<Terrain::RoadType> * Terrain::tile_turn() const {
  return &tile_turn_;
}

#endif

#ifndef ASSIGN3_TERRAIN_H_
#define ASSIGN3_TERRAIN_H_

#include <vector>
#include <string>
#include <cassert>
#include <queue>
#include <unordered_map>

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
    // Construct with width and height specified
    Terrain(const GLuint &program_id, const GLuint &water_id, const int &width = 32, const int &height = 32);

    // // Render the scene (all member models)
    // void Render();

    // THe texture for water
    GLuint water_texture_;

    GLuint terrain_water_id_;

    // The VAO Handle for the water
    unsigned int water_vao_handle_;

    unsigned int indice_count_water_;
    
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
    inline std::queue<std::unordered_map<float,std::pair<float,float>>> collision_queue_hash() const;
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
    // Width of the heightmap
    unsigned int x_length_;
    // Height of the heightmap
    unsigned int z_length_;
    // The amount of indices, used to render efficiently
    int indice_count_;
    // The amount of indices in a straight road piece, used to render efficiently
    int road_indice_count_;
    // The shader to use to render heightmap
    GLuint terrain_program_id_;
    // The texture to be used to Wrap Terrain
    GLuint texture_;
    // The texture to be used for the road
    GLuint road_texture_;
    
    // The VAO handle for the terrain
    // std::vector<unsigned int> terrain_vao_handle_;
    circular_vector<unsigned int> terrain_vao_handle_;
    // The VAO handle for the road
    // std::vector<unsigned int> road_vao_handle_;
    circular_vector<unsigned int> road_vao_handle_;
    // A queue representing each road tile for collision checking
    //   Each key in the map represents a Z scanline
    //   Each pair of values of the key represents it's min X and max X
    //     i.e. it's bounding box of the road
    //     pair.first = min_x, pair.second = max_x
    std::queue<std::unordered_map<float,std::pair<float,float>>> collision_queue_hash_;

    // GENERATE TERRAIN VARS
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coordinates_uv;
    std::vector<int> indices;
    // This vector is used to build heights and smooths previous tile connections
    std::vector<float> heights_; 
    // This vector is used only for water heightmap generation
    std::vector<float> water_heights_;
    // The current X,Z displacement from zero
    //   Used for joining tiles
    glm::vec2 next_tile_start_;
    // The previous maximum x vertice
    //   Used for updating next_tile_start_.x
    float prev_max_x_;
    // GENERATE ROAD VARS
    std::vector<glm::vec3> vertices_road_;
    std::vector<glm::vec3> normals_road_;
    std::vector<glm::vec2> texture_coordinates_uv_road_;
    std::vector<int> indices_road_;

    // Generates a random terrain piece and pushes it back into circular_vector VAO buffer
    void RandomizeGeneration();
    // TODO
    // Straight Terrain Piece
    //   Mutates all the input parameters for CreateVAO
    //   @warn creates and pushes back a road VAO based on the terrain middle section
    void GenerateTerrain();
    // TODO
    // Water height map
    //   Mutates all the input parameters for CreateVAO
    //   @warn creates and pushes back a water VAO
    //   @warn uses the indices and UV coordinates from terrain generation
    //         hence must be called after GenerateTerrain()
    void GenerateWater();
    // TODO
    // Turning Terrain Piece
    //   Mutates all the input parameters for CreateVAO
    //   @warn creates and pushes back a road VAO based on the terrain middle section
    void GenerateTerrainTurn();

    // TERRAIN GENERATION HELPERS
    void HelperMakeHeights();
    // Tile_type cases:
        // case 0: straight road => do nothing
        // case 1: x^2 turnning road
    void HelperMakeVertices(float min_position = 0.0f, float position_range = 20.0f, char road_type = 0, char tile_type = 0);
    void HelperMakeIndicesAndUV();
    void HelperMakeNormals();
    // ROAD GENERATION HELPERS
    void HelperMakeRoadVerticesAndNormals();
    void HelperMakeRoadVertices();
    void HelperMakeRoadNormals();
    void HelperMakeRoadIndicesAndUV();
    void HelperMakeRoadCollisionMap();

    // TODO
    unsigned int CreateVao(const GLuint &program_id);
    // TODO
    unsigned int CreateVao(const GLuint &program_id, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
        const std::vector<glm::vec2> &texture_coordinates_uv, const std::vector<int> &indices);
    // Creates a texture pointer from file
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
inline std::queue<std::unordered_map<float,std::pair<float,float>>> Terrain::collision_queue_hash() const {
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

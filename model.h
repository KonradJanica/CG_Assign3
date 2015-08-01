#ifndef ASSIGN3_MODEL_H_
#define ASSIGN3_MODEL_H_

#include "model_data.h"

#include "lib/stb_image/stb_image.h"

#include "utils/includes.h"

// A model to draw via opengl
//   Creates and stores VAO and Material data for rendering
//   @usage Object * car = new model(program_id, "car-n.obj")
class Model {
  public:
    // Enum for vertex coordinates
    //   used in GetMax
    enum CoordEnum {
      kX = 0,
      kY = 1, 
      kZ = 2,
      kx = 0, 
      ky = 1, 
      kz = 2,
      kMin = 3,
      kMax = 3,
    };  

    Model(const Shader & shader, const std::string &model_filename);

    // Accessor for current shader program.
    //   @return program_id_, the shader used by the model
    inline const Shader * shader() const;

    // Accessor for each shapes VAO and it's corresponding texture
    //   Each VAO includes indices, vertices and UV coordinates
    //   @return vao_texture_handle_, a container for all VAOs and their corresponding textures
    inline const std::vector<std::pair<GLuint, GLuint> > * vao_texture_handle() const;

    // Accessor for largest vertex
    //   @param enum value
    //   @return max_$_, the maximum cartesian coordinate of given input
    inline float GetMax( int e_numb ) const;

    // Accessor for smallest vertex
    //   @param enum value
    //   @return min_$_, the minimum cartesian coordinate of given input
    inline float GetMin( int e_numb ) const;

    // Accessor for the points of a given VAO shape
    //   @param index of shape
    //   @return unsigned int, amount of points in the VAO shape
    //   @warn throws exception on error
    inline unsigned int points_per_shape_at(unsigned int x) const;

    // Accessor for the Ambient Surface Colours vector
    //   @param index of colour
    //   @return ambient_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    inline glm::vec3 ambient_surface_colours_at(unsigned int index) const;

    // Accessor for the Diffuse Surface Colours vector
    //   @param index of colour
    //   @return diffuse_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    inline glm::vec3 diffuse_surface_colours_at(unsigned int index) const;

    // Accessor for the Specular Surface Colours vector
    //   @param index of colour
    //   @return glm::vec3, a vec3 corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    inline glm::vec3 specular_surface_colours_at(unsigned int index) const;

    // Accessor for the Shininess vector
    //   @param index of shininess
    //   @return float, a float corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    inline float shininess_at(unsigned int index) const;

    // Accessor for the Dissolve vector
    //   @param index of dissolve vector
    //   @return float, a float corresponding to the vao_texture_handle index
    //   @warn throws exception on error
    inline float dissolve_at(unsigned int index) const;

    // Accessor for all of the points in the shape
    //   @return amount_points_, the total amount of points => used for rendering
    inline unsigned int amount_points() const;

  private:
    // Shader program
    const Shader &shader_;
    // RawModelData
    ModelData *model_data_;
    // The file path to the mtl file (and hopefully the textures)
    std::string subdir_;
    // Each pair is a VAO with its texture
    std::vector<std::pair<GLuint, GLuint> > vao_texture_handle_;
    // Each index represents the points per shape in vao_texture_handle_
    std::vector<unsigned int> points_per_shape_;
    // The Ambient Surface Colour per vao_texture_handle_
    std::vector<glm::vec3> ambient_surface_colours_;
    // The Diffuse Surface Colours per vao_texture_handle_
    std::vector<glm::vec3> diffuse_surface_colours_;
    // The Specular Surface Colours per vao_texture_handle_
    std::vector<glm::vec3> specular_surface_colours_;
    // The Shininess of the normal per vao_texture_handle_
    std::vector<float> shininess_;
    // The Dissolve of the texture per vao_texture_handle_
    std::vector<float> dissolve_;
    // Amount of points of shape in total
    unsigned int amount_points_;
    // Members contain min/max of the cartesian coordinates of the model
    float max_x_;
    float max_y_;
    float max_z_;
    float min_x_;
    float min_y_;
    float min_z_;
    // Overall Min/Max
    float min_;
    float max_;

    //Constructor Helpers
    void ConstructShadedModel();
    unsigned int CreateVao(const RawModelData::Shape *shape);
    GLuint CreateTextures(const RawModelData::Material *material);
};

// Returns the program_id_ (i.e. the shader) that the model uses
//   @return program_id_, the shader member variable
inline const Shader * Model::shader() const {
  return &shader_;
}

// Accessor for the VAO & Texture Handler
//   @return vao_texture_handle_, the handler for all VAOS for the shape and their textures
inline const std::vector<std::pair<unsigned int, GLuint> > * Model::vao_texture_handle() const {
  return &vao_texture_handle_;
}

// Accessor for largest x vertex. Sample Usage:
//   float max = model->GetMax(model::kX)
//   @param enum value
//   @return max_$_, the maximum cartesian coordinate of given input
inline float Model::GetMax( int e_numb ) const {
  switch(e_numb) {
    case 0: 
      return max_x_;
    case 1: 
      return max_y_;
    case 2:
      return max_z_;
    case 3:
      return max_;
    default:
      assert(false);
  }
}

// Accessor for smallest x vertex. Sample Usage:
//   float min = GetMin(X)
//   @param enum value
//   @return min_$_, the minimum cartesian coordinate of given input
inline float Model::GetMin( int e_numb ) const {
  switch(e_numb) {
    case 0: 
      return min_x_;
    case 1: 
      return min_y_;
    case 2:
      return min_z_;
    case 3:
      return min_;
    default:
      assert(false);
  }
}

// Accessor for the points of a given VAO shape. Sample Usage:
//   unsigned int points_to_render = model->pointers_per_shape_at(0)
//   @param index of shape
//   @return unsigned int, amount of points in the VAO shape
//   @warn throws exception on error
inline unsigned int Model::points_per_shape_at(unsigned int x) const {
  assert(x < points_per_shape_.size() && "Trying to access vector out of bounds, this shape didn't push into points_per_shape");
  return points_per_shape_.at(x);
}

// Accessor for the Ambient Surface Colours vector. Sample Usage:
//   glm::vec3 ambient = model->ambient_surface_colours_at(3)
//   @param index of colour
//   @return ambient_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
//   @warn throws exception on error
inline glm::vec3 Model::ambient_surface_colours_at(unsigned int index) const {
  assert(index < ambient_surface_colours_.size() && "Trying to access vector out of bounds");
  return ambient_surface_colours_.at(index);
}

// Accessor for the Diffuse Surface Colours vector. Sample Usage:
//   glm::vec3 diffuse = model->diffuse_surface_colours_at(3)
//   @param index of colour
//   @return diffuse_surface_colours_, a vector of vec3 corresponding to the vao_texture_handle index
//   @warn throws exception on error
inline glm::vec3 Model::diffuse_surface_colours_at(unsigned int index) const {
  assert(index < diffuse_surface_colours_.size() && "Trying to access vector out of bounds");
  return diffuse_surface_colours_.at(index);
}

// Accessor for the Specular Surface Colours vector. Sample Usage:
//   glm::vec3 specular = model->specular_surface_colours_at(3)
//   @param index of colour
//   @return glm::vec3, a vec3 corresponding to the vao_texture_handle index
//   @warn throws exception on error
inline glm::vec3 Model::specular_surface_colours_at(unsigned int index) const {
  assert(index < specular_surface_colours_.size() && "Trying to access vector out of bounds");
  return specular_surface_colours_.at(index);
}

// Accessor for the Shininess vector. Sample Usage:
//  float shininess = model->shininess_at(3)
//   @param index of shininess
//   @return float, a float corresponding to the vao_texture_handle index
//   @warn throws exception on error
inline float Model::shininess_at(unsigned int index) const {
  assert(index < shininess_.size() && "Trying to access vector out of bounds");
  return shininess_.at(index);
}

// Accessor for the Dissolve vector. Sample Usage:
//  float dis = model->dissolve_at(3)
//   @param index of dissolve vector
//   @return float, a float corresponding to the vao_texture_handle index
//   @warn throws exception on error
inline float Model::dissolve_at(unsigned int index) const {
  assert(index < dissolve_.size() && "Trying to access vector out of bounds");
  return dissolve_[index];
}

// Accessor for all of the points in the shape. Sample Usage:
//   unsigned int amount_render = model->amount_points()
//   @return amount_points_, the total amount of points => used for rendering
inline unsigned int Model::amount_points() const {
  return amount_points_;
}
#endif

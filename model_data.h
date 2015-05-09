#ifndef ASSIGN2_MODEL_MODELDATA_H_
#define ASSIGN2_MODEL_MODELDATA_H_

#include <vector>
#include <string>
#include <iostream>

#include "glm/glm.hpp"

#include "lib/tiny_obj_loader/tiny_obj_loader.h"

// Stores the Data from OBJ files. Sample Usage:
//  RawModelData.shapes[3].vertices[1].x
//  Should be abstracted
struct RawModelData {

  struct Shape {
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coordinates_uv;

    int material_id;
  };

  struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 transmittance;
    glm::vec3 emission;
    float shininess;
    float ior;
    float dissolve;
    float illum;  
    // These will be empty if texture img doesn't exist
    std::string ambient_texture;
    std::string diffuse_texture;
    std::string specular_texture;
    std::string normal_texture;
  };

  std::vector<Shape*> shapes;
  std::vector<Material*> materials;
};

class ModelData {

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
    };  
    
    // Parses raw OBJ file into a ModelData object
    //   @param .obj model file
    ModelData(const std::string& inputfile);

    // Adds More Shapes/Materials to ModelData. Sample Usage:
    //   AddData("models/cube.obj)
    //   @param .obj model file 
    void AddData(const std::string& inputfile);

    // Accessor for shapes in model_data_. Sample Usage:
    //   Shape* new_shape = model.shape_at(index)
    //   @param required index of shape vector
    //   @return shape pointer at index
    RawModelData::Shape* shape_at(unsigned int index) const;

    // Accessor for materials in model_data_. Sample Usage:
    //   Material* new_material = model.material_at(index)
    //   @param required index of material vector
    //   @return material pointer at index
    RawModelData::Material* material_at(unsigned int index) const;

    // Accessor for largest x vertex. Sample Usage:
    //   float max = GetMax(X)
    //   @param enum value
    float GetMax(int e_numb);

    // Accessor for smallest x vertex. Sample Usage:
    //   float min = GetMin(X)
    //   @param enum value
    float GetMin(int e_numb);
    
  private:
    RawModelData model_data_;
    float max_x_;
    float max_y_;
    float max_z_;
    float min_x_;
    float min_y_;
    float min_z_;
};

#endif

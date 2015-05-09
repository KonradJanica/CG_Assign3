#include "model_data.h"

ModelData::ModelData(const std::string& inputfile) : max_x_(0), max_y_(0), max_z_(0) {

  AddData(inputfile);

}

void ModelData::AddData(const std::string& inputfile) {

  // std::string inputfile = "models/cube-simple.obj";

  std::string subdir = inputfile.substr(0, inputfile.find_last_of('/') + 1);

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err = tinyobj::LoadObj(shapes, materials, inputfile.c_str(), subdir.c_str());

  if (!err.empty()) {
    std::cerr << err << std::endl;
    exit(1);
  }

  model_data_.shapes.reserve(shapes.size());
  model_data_.materials.reserve(materials.size());
   //std::cout << "# of shapes    : " << shapes.size() << std::endl;
   //std::cout << "# of materials : " << materials.size() << std::endl;
  glm::vec3 new_vertice;
  glm::vec2 new_uv;
  glm::vec3 new_normal;

  for (size_t i = 0; i < shapes.size(); i++) {
    RawModelData::Shape* new_shape = new RawModelData::Shape();
    new_shape->indices.reserve(shapes[i].mesh.indices.size());
    new_shape->vertices.reserve(shapes[i].mesh.positions.size() / 3);
    new_shape->normals.reserve(shapes[i].mesh.normals.size() / 3);
    new_shape->normals.resize(shapes[i].mesh.normals.size() / 3);
    //std::cout << "Shape = " << i << " vertices = " << shapes[i].mesh.positions.size() << " normals = " << shapes[i].mesh.normals.size() << std::endl;
    // printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
    // printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
    // printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
    assert((shapes[i].mesh.indices.size() % 3) == 0);
    for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
      new_shape->indices.push_back(shapes[i].mesh.indices[3*f+0]);
      new_shape->indices.push_back(shapes[i].mesh.indices[3*f+1]);
      new_shape->indices.push_back(shapes[i].mesh.indices[3*f+2]);
      // printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, shapes[i].mesh.indices[3*f+0], shapes[i].mesh.indices[3*f+1], shapes[i].mesh.indices[3*f+2], shapes[i].mesh.material_ids[f]);
    }
    // printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
    assert((shapes[i].mesh.positions.size() % 3) == 0);
    for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
      // printf("  v[%ld] = (%f, %f, %f)\n", v,
      //     shapes[i].mesh.positions[3*v+0],
      //     shapes[i].mesh.positions[3*v+1],
      //     shapes[i].mesh.positions[3*v+2]);
      // Vertices
      new_vertice.x = shapes[i].mesh.positions[3*v+0]; 
      new_vertice.y = shapes[i].mesh.positions[3*v+1];
      new_vertice.z = shapes[i].mesh.positions[3*v+2];
      new_shape->vertices.push_back(new_vertice);
      //  std::cout << new_vertice.x << std::endl;
      //  std::cout << new_vertice.y << std::endl;
      //  std::cout << new_vertice.z << std::endl;
      if (new_vertice.x > max_x_)
        max_x_ = new_vertice.x;
      if (new_vertice.y > max_y_)
        max_y_ = new_vertice.y;
      if (new_vertice.z > max_z_)
        max_z_ = new_vertice.z;
      if (new_vertice.x < min_x_)
        min_x_ = new_vertice.x;
      if (new_vertice.y < min_y_)
        min_y_ = new_vertice.y;
      if (new_vertice.z < min_z_)
        min_z_ = new_vertice.z;
    }
    for (size_t n = 0; n < shapes[i].mesh.normals.size() / 3; n++) {
      new_normal.x = shapes[i].mesh.normals[3*n+0];
      new_normal.y = shapes[i].mesh.normals[3*n+1];
      new_normal.z = shapes[i].mesh.normals[3*n+2];

      new_shape->normals.at(n) = new_normal;
    }
    //for (size_t x=0; x<shapes[i].mesh.texcoords.size(); x+=2) {
    //  new_uv.x = shapes[i].mesh.texcoords.at(x);
    //  new_uv.y = shapes[i].mesh.texcoords.at(x+1);
    //  // std::cout << "vt " << new_uv.x << " " << new_uv.y << std::endl;
    //  new_shape->texture_coordinates_uv.push_back(new_uv);
    //}
    for (size_t t = 0; t < shapes[i].mesh.texcoords.size() / 2; t++ ) {
      new_uv.x = shapes[i].mesh.texcoords[2*t+0];
      // Stbi_image_load loads the picture starting top-left  whereas
      //   opengl renders using lower left, hence inverting y will fix this
      new_uv.y = 1.0-shapes[i].mesh.texcoords[2*t+1];
    
      new_shape->texture_coordinates_uv.push_back(new_uv);
    }
    //printf("Shapes = %d, Material_ids.size = %d\n",i,shapes[i].mesh.material_ids.size());
    //for (unsigned int x = 0; x < shapes[i].mesh.material_ids.size(); ++x ) {
    //  printf("%d,",shapes[i].mesh.material_ids.at(x));
    //}
    //printf("\n\n");

    if ( shapes[i].mesh.material_ids.size() > 0 ) {
      new_shape->material_id = shapes[i].mesh.material_ids.at(0);
    }
    else {
      new_shape->material_id = -1;
    }

    model_data_.shapes.push_back(new_shape);
  }
  for (size_t i = 0; i < shapes.size(); ++i) {
      // printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
      // printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
      // printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
      // printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
      // printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
      // printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
      // printf("  material.Ns = %f\n", materials[i].shininess);
      // printf("  material.Ni = %f\n", materials[i].ior);
      // printf("  material.dissolve = %f\n", materials[i].dissolve);
      // printf("  material.illum = %d\n", materials[i].illum);
        //printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
        //printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
        //printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
        //printf("  material.map_Ns = %s\n", materials[i].normal_texname.c_str());
      // std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
      // std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
      // for (; it != itEnd; it++) {
      //    printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
      // }
      //  printf("\n");

      RawModelData::Material* new_material = new RawModelData::Material();
      new_material->ambient_texture = materials[i].ambient_texname;  // Will be empty if texture img doesn't exist
      new_material->diffuse_texture = materials[i].diffuse_texname;  // Will be empty if texture img doesn't exist
      new_material->specular_texture = materials[i].specular_texname;  // Will be empty if texture img doesn't exist
      new_material->normal_texture = materials[i].normal_texname;  // Will be empty if texture img doesn't exist
       //std::cout << "Material id = " << i << " Texture name = " << new_material->diffuse_texture << std::endl;
      model_data_.materials.push_back(new_material);

      new_material->ambient.x = materials[i].ambient[0];
      new_material->ambient.y = materials[i].ambient[1];
      new_material->ambient.z = materials[i].ambient[2];

      new_material->diffuse.x = materials[i].diffuse[0];
      new_material->diffuse.y = materials[i].diffuse[1];
      new_material->diffuse.z = materials[i].diffuse[2];

      new_material->specular.x = materials[i].specular[0];
      new_material->specular.y = materials[i].specular[1];
      new_material->specular.z = materials[i].specular[2];

      new_material->transmittance.x = materials[i].transmittance[0];
      new_material->transmittance.y = materials[i].transmittance[1];
      new_material->transmittance.z = materials[i].transmittance[2];

      new_material->emission.x = materials[i].emission[0];
      new_material->emission.y = materials[i].emission[1];
      new_material->emission.z = materials[i].emission[2];

      new_material->shininess = materials[i].shininess;
      new_material->ior = materials[i].ior;
      new_material->dissolve = materials[i].dissolve;
      new_material->illum = materials[i].illum;
    }
}

// Returns the required shape pointer in the member vector
// If no shape exists at given index, returns NULL
//  @example Shape* next_shape = model_data.shape_at(0)
//  @param unsigned int index of vector
//  @return shape* at index, null otherwise
RawModelData::Shape* ModelData::shape_at(unsigned int index) const {
  if (index > model_data_.shapes.size() - 1)
    return NULL;
  
  return model_data_.shapes.at(index);
}

// Returns the required material pointer in the member vector
// If no material exists at given index, returns NULL
//  @example Material* next_material = model_data.material_at(0)
//  @param unsigned int index of vector
//  @return material* at index, null otherwise
RawModelData::Material* ModelData::material_at(unsigned int index) const {
  if (index > model_data_.shapes.size() - 1)
    return NULL;
  
  return model_data_.materials.at(index);
}

float ModelData::GetMax(int e_numb) {
  switch(e_numb) {
    case 0: 
      return max_x_;
    case 1: 
      return max_y_;
    case 2:
      return max_z_;
    default:
      assert(false);
  }
}

float ModelData::GetMin(int e_numb) {
  switch(e_numb) {
    case 0: 
      return min_x_;
    case 1: 
      return min_y_;
    case 2:
      return min_z_;
    default:
      assert(false);
  }
}

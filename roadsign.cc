#include "roadsign.h"

RoadSign::RoadSign(const Shaders * shaders, const Terrain * terrain) :
  // Reference objects
  shaders_(shaders),
  terrain_(terrain),
  // Make road signs
  signs_{(AddModel(shaders, "models/Signs_OBJ/working/60.obj")),
        (AddModel(shaders, "models/Signs_OBJ/working/curve_left.obj")),
        (AddModel(shaders, "models/Signs_OBJ/working/curve_right.obj"))} {

    active_signs_.assign(3, -1);

  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_left.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_right.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/60.obj");
        }

Object * RoadSign::SignSpawn() {
  for (unsigned int x = 0; x < signs_.size(); ++x) {
    // Check if sign can be used
    if (active_signs_[x] < 0) {
      const auto turn_type_vec = terrain_->tile_turn();
      // Get last turn type
      const auto turn_type = turn_type_vec->back();
      // Check if turn type suits - @note requires same indexing as enum
      if (x == turn_type) {
        // Put sign before turn begins
        unsigned int index = turn_type_vec->size()-1;
        active_signs_[x] = index;
        // printf("setting ind = %d\n",active_signs_[x]);
        // printf("size = %d\n",turn_type_vec->size());
        // Get middle of terrain tile
        const circular_vector<Terrain::colisn_vec> * road_colisn_pairs = terrain_->colisn_boundary_pairs();
        const Terrain::colisn_vec &colisn_vector = (*road_colisn_pairs)[index];
        const Terrain::boundary_pair &mid_tile = colisn_vector[colisn_vector.size()/5]; // 1/5 thru road tile (tile starts from back)
        // printf("road_colisn size = %d\n", road_colisn_pairs->size());
        // Get direction pointing to cliff side of road
        glm::vec3 dir = mid_tile.second - mid_tile.first;
        // Get placement point from direction and point nearest cliff
        glm::vec3 placement_point = mid_tile.second + glm::vec3(dir.x/2.0f, 0.0f, dir.z/2.0f);
        // glm::vec3 placement_point = mid_tile.second;
        dir = glm::normalize(dir);
        const glm::vec2 horiz_plane = glm::vec2(dir.x, dir.z);
        const float rot_y = glm::orientedAngle(horiz_plane, glm::vec2(0.0f,1.0f));

        // printf("placement_point = %f\n",placement_point.z);
        // Reset sign and return
        signs_[x]->set_translation(placement_point);
        int xx = rand() % 21 - 10;
        int yy = rand() % 11;
        int zz = rand() % 21 - 10;
        signs_[x]->set_rotation(glm::vec3(xx,rot_y-90.0f+yy,zz));
        signs_[x]->UpdateModelMatrix();
        return signs_[x];
      }
    }
  }
  // No road signs available, return null
  return 0;
}

// Moves the active signs indexes
//   Reactivates past signs
void RoadSign::ShiftIndexes() {
  // int x = 0;
  for (auto it = active_signs_.begin();
      it != active_signs_.end(); ++it) {
    if (*it > -1) //prevent overflow
      (*it)--;
    // printf("active_signs_[%d] = %d\n", x, (*it));
    // x++;
  }
}

// Creates a model for the member
//   @param shader, a shader program
//   @param model_filename, a string containing the path of the .obj file
Object * RoadSign::AddModel(const Shaders * shaders, const std::string &model_filename) const {
  const Shader &shader = shaders->LightMappedGeneric;
  Object * object = new Object(shader, model_filename,
      glm::vec3(2.2f, 0.0f, 50.0f), // Translation
      glm::vec3(0.0f, 20.0f, 0.0f), // Rotation
      glm::vec3(0.9f, 0.9f*1.3f, 0.9f)); // Scale

  return object;
}


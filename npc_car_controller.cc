#include "npc_car_controller.h"

NpcCarController::NpcCarController(const Shaders * shaders,
                   const Terrain * terrain,
                   const Renderer * renderer,
                   const Camera * camera,
                   const Sun * sun) :
  // Reference objects
  shaders_(shaders),
  terrain_(terrain),
  renderer_(renderer),
  camera_(camera),
  sun_(sun),
  // Make road signs
  cars_{AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj"),
        AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj"),
        AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj")} {

    active_cars_.assign(3, -1);

  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_left.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_right.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/60.obj");
        }

Car * NpcCarController::SignSpawn() {
  for (unsigned int x = 0; x < cars_.size(); ++x) {
    // Check if sign can be used
    if (active_cars_[x] < 0) {
      const auto turn_type_vec = terrain_->tile_turn();
      // Get last turn type
      const auto turn_type = turn_type_vec->back();
      // Check if turn type suits - @note requires same indexing as enum
      if (x == turn_type) {
        // Put sign before turn begins
        unsigned int index = turn_type_vec->size()-1;
        active_cars_[x] = index;
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
        cars_[x]->set_translation(placement_point);
        int xx = rand() % 21 - 10;
        int yy = rand() % 11;
        int zz = rand() % 21 - 10;
        cars_[x]->set_rotation(glm::vec3(xx,rot_y-90.0f+yy,zz));
        cars_[x]->UpdateModelMatrix();
        return cars_[x];
      }
    }
  }
  // No road signs available, return null
  return 0;
}

// Moves the active signs indexes
//   Reactivates past signs
void NpcCarController::ShiftIndexes() {
  // int x = 0;
  for (auto it = active_cars_.begin();
      it != active_cars_.end(); ++it) {
    if (*it > -1) //prevent overflow
      (*it)--;
    // printf("active_signs_[%d] = %d\n", x, (*it));
    // x++;
  }
}

// Update physics of all cars
void NpcCarController::UpdateCars(float delta_time) {
  for (Car * c : cars_) {
    c->ControllerMovementTick(delta_time);
  }
}

// Render all roadsigns
void NpcCarController::DrawCars() const {
  for (Car * c : cars_) {
    renderer_->Render(c, *camera_, *sun_);
  }
}

// Creates a model for the member
//   @param shader, a shader program
//   @param model_filename, a string containing the path of the .obj file
Car * NpcCarController::AddCar(const std::string &model_filename) const {
  const Shader &shader = shaders_->LightMappedGeneric;
  Car * car = new Car(shader, model_filename,
      glm::vec3(0.95f, 0.55f, 35.0f),     // Translation  move behind first tile (i.e. start on 2nd tile)
      glm::vec3(0.0f,  0.0f, 0.0f),       // Rotation
      glm::vec3(0.4f,  0.4f*1.6f, 0.4f),  // Scale
      10, false); // starting speed and debugging mode

  return car;
}


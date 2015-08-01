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
  // Make cars
  cars_{AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj"),
        AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj"),
        AddCar("models/Pick-up_Truck/pickup_wind_alpha.obj")} {


    // Initial npc car states
    cars_direction_.assign(cars_.size(), false);
    collision_controllers_.reserve(cars_.size());
    for (unsigned int x = 0; x < cars_.size(); ++x) {
      collision_controllers_.push_back(new CollisionController());
      Car * c = cars_[x];
      RespawnCar(c, x);
    }

  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_left.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_right.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/60.obj");
        }

// Update physics of all cars
kGameState NpcCarController::UpdateCars(float delta_time, kGameState current_state) {
  // for (Car * c : cars_) {
    // c->ControllerMovementTick(delta_time);
  // }

    // printf("HERE\n");
  for (unsigned int x = 0; x < cars_.size(); ++x) {
    Car * c = cars_[x];
    CollisionController * cc = collision_controllers_[x];

    if (cars_direction_[x]) {
      cc->UpdateCollisionsNPC(c, terrain_, current_state);
    } else {
      cc->UpdateCollisionsNPCReverse(c, terrain_, current_state);
    }

    if (cc->dis() > 10.0f)
      if (rand() % 10 < 3)
        RespawnCar(c, x);

    cc->AutoDrive(c, delta_time);

    c->UpdateModelMatrix();
  }

  return current_state; //TODO does nothing
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
      60, false); // starting speed and debugging mode

  return car;
}

// Respawn car randomly in front or back of terrain
//  Notes this cars direction in cars_direction_ vector
//  Creates a new CollisionController for each car
//  TODO collision spawn check (ensure no stacked spawn)
void NpcCarController::RespawnCar(Car * car, int car_index) {
  bool random = rand() % 2;
  glm::vec3 spawn_point;
  if (random) {
    spawn_point = terrain_->colisn_boundary_pair_first();
    collision_controllers_[car_index]->Reset(true, terrain_);
  } else {
    spawn_point = terrain_->colisn_boundary_pair_last();
    collision_controllers_[car_index]->Reset(false, terrain_);
  }
  spawn_point.y = car->default_height();
  car->set_translation(spawn_point);
  
  // printf("car bool = %d\n", random);
  cars_direction_[car_index] = random;
}

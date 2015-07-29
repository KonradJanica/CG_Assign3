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
      60, false); // starting speed and debugging mode

  return car;
}


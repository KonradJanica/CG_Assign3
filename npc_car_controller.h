#ifndef ASSIGN3_NPCCARCONTROLLER_H_
#define ASSIGN3_NPCCARCONTROLLER_H_

#include "terrain.h"
#include "car.h"
#include "collision_controller.h"
#include "renderer.h"
#include "camera.h"

#include "utils/includes.h"
#include "utils/constants.h"

class NpcCarController {
  public:
    NpcCarController(const Shaders * shaders,
             const Terrain * terrain,
             const Renderer * renderer,
             const Camera * camera);

    // Update physics of all cars
    kGameState UpdateCars(float delta_time, kGameState current_state);

    // Render all roadsigns
    void DrawCars() const;

    // Accessor for the signs
    //   Used for rendering the signs
    inline std::vector<Car*> cars() const;
    // Accessor for the Collision Controllers
    //   Used for decrementing the vector indices
    inline std::vector<CollisionController*> collision_controllers() const;

  private:
    const Shaders * shaders_;
    const Terrain * terrain_;
    const Renderer * renderer_;
    const Camera * camera_;

    // The NPC Cars
    std::vector<Car*> cars_;
    // Their assosicated collision controllers
    std::vector<CollisionController*> collision_controllers_;

    // The direction of the cars (in above order)
    //   index = order, value: true = forward, false = reverse
    std::vector<bool> cars_direction_;

    Car * AddCar(const std::string &file_name) const;

    // Respawn car randomly in front or back of terrain
    //  Notes this cars direction in cars_direction_ vector
    //  TODO collision spawn check (ensure no stacked spawn)
    void RespawnCar(Car * car, int car_index);
};

// Accessor for the signs
//   Used for rendering the signs
inline std::vector<Car*> NpcCarController::cars() const {
  return cars_;
}
// Accessor for the Collision Controllers
//   Used for decrementing the vector indices
inline std::vector<CollisionController*> NpcCarController::collision_controllers() const {
  return collision_controllers_;
}

#endif

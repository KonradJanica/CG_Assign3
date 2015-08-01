#ifndef ASSIGN3_NPCCARCONTROLLER_H_
#define ASSIGN3_NPCCARCONTROLLER_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "terrain.h"
#include "car.h"
#include "collision_controller.h"
#include "renderer.h"
#include "camera.h"
#include "sun.h"

#include "constants.h"

#include "glm/glm.hpp"
#include "shaders/shader_compiler/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h>
#endif

class NpcCarController {
  public:
    NpcCarController(const Shaders * shaders,
             const Terrain * terrain,
             const Renderer * renderer,
             const Camera * camera,
             const Sun * sun);

    // Update physics of all cars
    kGameState UpdateCars(float delta_time, kGameState current_state);

    // Render all roadsigns
    void DrawCars() const;

    // Accessor for the signs
    //   Used for rendering the signs
    inline std::vector<Car*> cars() const;
    // Accessor for the signs that are active
    //   Used to optimize draw
    inline std::vector<int> active_cars() const;
    // Accessor for the Collision Controllers
    //   Used for decrementing the vector indices
    inline std::vector<CollisionController*> collision_controllers() const;

  private:
    const Shaders * shaders_;
    const Terrain * terrain_;
    const Renderer * renderer_;
    const Camera * camera_;
    const Sun * sun_;

    // The NPC Cars
    std::vector<Car*> cars_;
    // Their assosicated collision controllers
    std::vector<CollisionController*> collision_controllers_;

    // The active cars (in above order)
    //   index = order, value = index in circular_vector
    std::vector<int> active_cars_;

    Car * AddCar(const std::string &file_name) const;

    // Respawn car randomly in front or back of terrain
    //  TODO collision spawn check (ensure no stacked spawn)
    void RespawnCar(Car * car);

};

// Accessor for the signs
//   Used for rendering the signs
inline std::vector<Car*> NpcCarController::cars() const {
  return cars_;
}
// Accessor for the signs that are active
//   Used to optimize draw
inline std::vector<int> NpcCarController::active_cars() const {
  return active_cars_;
}
// Accessor for the Collision Controllers
//   Used for decrementing the vector indices
inline std::vector<CollisionController*> NpcCarController::collision_controllers() const {
  return collision_controllers_;
}

#endif

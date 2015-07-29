#ifndef ASSIGN3_NPCCOLLISIONCONTROLLER_H_
#define ASSIGN3_NPCCOLLISIONCONTROLLER_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "terrain.h"
#include "car.h"
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

class NpcCollisionController {
  public:
    NpcCollisionController(const Shaders * shaders,
             const Terrain * terrain,
             const Renderer * renderer,
             const Camera * camera,
             const Sun * sun);

    // TODO comment, make const
    Car * SignSpawn();

    // Moves the active signs indexes
    //   Reactivates past signs
    void ShiftIndexes();

    // Render all roadsigns
    void DrawSigns() const;

    // Accessor for the signs
    //   Used for rendering the signs
    inline std::vector<Car*> signs() const;
    // Accessor for the signs that are active
    //   Used to optimize draw
    inline std::vector<int> active_signs() const;

  private:
    const Shaders * shaders_;
    const Terrain * terrain_;
    const Renderer * renderer_;
    const Camera * camera_;
    const Sun * sun_;

    // The actual road sign objects
    //   The indexing is as listed below
    // sign_60_;
    // sign_left_;
    // sign_right_;
    std::vector<Car*> signs_;

    // The active signs (in above order)
    //   index = order, value = index in circular_vector
    std::vector<int> active_signs_;

    Car * AddModel(const Shaders * shaders, const std::string &file_name) const;

};

// Accessor for the signs
//   Used for rendering the signs
inline std::vector<Car*> NpcCollisionController::signs() const {
  return signs_;
}
// Accessor for the signs that are active
//   Used to optimize draw
inline std::vector<int> NpcCollisionController::active_signs() const {
  return active_signs_;
}

#endif

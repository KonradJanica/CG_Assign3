#ifndef ASSIGN3_ROADSIGN_H_
#define ASSIGN3_ROADSIGN_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "terrain.h"
#include "object.h"

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

class RoadSign {
  public:
    RoadSign(const Shaders * shaders, const Terrain * terrain);

    // TODO comment, make const
    Object * SignSpawn();

    // Moves the active signs indexes
    //   Reactivates past signs
    void ShiftIndexes();

    // Accessor for the signs
    //   Used for rendering the signs
    inline std::vector<Object*> signs() const;
    // Accessor for the signs that are active
    //   Used to optimize draw
    inline std::vector<int> active_signs() const;

  private:
    const Shaders * shaders_;
    const Terrain * terrain_;

    // The actual road sign objects
    //   The indexing is as listed below
    // sign_60_;
    // sign_left_;
    // sign_right_;
    std::vector<Object*> signs_;

    // The active signs (in above order)
    //   index = order, value = index in circular_vector
    std::vector<int> active_signs_;

    Object * AddModel(const Shaders * shaders, const std::string &file_name) const;

};

// Accessor for the signs
//   Used for rendering the signs
inline std::vector<Object*> RoadSign::signs() const {
  return signs_;
}
// Accessor for the signs that are active
//   Used to optimize draw
inline std::vector<int> RoadSign::active_signs() const {
  return active_signs_;
}

#endif

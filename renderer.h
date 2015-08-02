#ifndef ASSIGN3_RENDERER_H_
#define ASSIGN3_RENDERER_H_

#include "model_data.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"
#include "object.h"
#include "Skybox.h"
#include "Water.h"

#include "utils/includes.h"

// The Class used for rendering
//   All DrawElement etc. openGL calls are made through this class
//   Everything should be const to maximize read only performance
class Renderer {
  public:
    // Construct with verbose debugging mode option
    Renderer(const bool debug_flag = false);

    // Draws/Renders the passed in objects (with their models) to the scene
    //   @param Object * object, an object to render
    //   @param Camera * camera, a camera to build mvp matrices
    void Render(const Object * object, const Camera &camera) const;
    // Draws/Renders the passed in terrain to the scene
    //   @param Terrain * terrain, a terrain (cliffs/roads) to render
    //   @param Camera * camera, a camera to build mvp matrices
    void Render(const Terrain * terrain, const Camera &camera) const;
    // Render Coordinate Axis 
    //   Only renders in debugging mode
    void RenderAxis(const Camera &camera) const;
    // Enable x,y,z axis coordinates VAO
    //   @return a VAO to use for the Axis
    GLuint EnableAxis() const;

    void RenderWater(const Water * water, const Camera &camera) const;

    void RenderSkybox(const Skybox * Sky, const Camera &camera) const;

    // Accessor for a shaders pointer
    inline const Shaders * shaders() const;

  private:
    // The shaders object (holds and compiles all shaders)
    const Shaders shaders_;

    // The VAO Handle for the Axis Coordinates
    const GLuint coord_vao_handle_;

    // Verbose Debugging mode
    const bool is_debugging_;
};

// Accessor for a shaders pointer
inline const Shaders * Renderer::shaders() const {
  return &shaders_;
}

#endif

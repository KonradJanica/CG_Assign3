#ifndef ASSIGN3_RENDERER_H_
#define ASSIGN3_RENDERER_H_

#include <vector>
#include <string>
#include <cassert>
#include "model_data.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"
#include "object.h"
#include "Skybox.h"
#include "Water.h"
#include "shaders/shaders.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Renderer {
  public:
    // Construct with a camera and verbose debugging mode
    Renderer(const Camera * camera, const bool debug_flag = false);

    // Draws/Renders the passed in objects (with their models) to the scene
    //   @param Object * object, an object to render
    //   @warn this function is not responsible for NULL PTRs
    //   @warn uses camera pointer for view matrix
    void Render(const Object * object) const;
    // Draws/Renders the passed in terrain to the scene
    //   @param Terrain * terrain, a terrain (cliffs/roads) to render
    //   @warn uses camera pointer for view matrix
    void Render(const Terrain * terrain) const;
    // TODO
    void RenderDepthBuffer(const Terrain * terrain) const;
    // Render Coordinate Axis 
    //   @warn requires VAO from EnableAxis
    //   @warn uses camera pointer for view matrix
    void RenderAxis() const;
    // Enable x,y,z axis coordinates
    void EnableAxis();

    void RenderWater(const Water * water, const Object * object, const Skybox * Sky) const;

    void RenderTerrain();

    void RenderSkybox(const Skybox * Sky) const;

    // Accessor for a shaders pointer
    inline const Shaders * shaders() const;

    // The Depth buffer texture index
    GLuint depth_texture_;
    // The Frame buffer to hold the Depth buffer
    GLuint frame_buffer_name_;

  private:
    // The shaders object (holds and compiles all shaders)
    const Shaders shaders_;
    // The camera to get view matrix
    const Camera * camera_;


    // The VAO Handle for the Axis Coordinates
    unsigned int coord_vao_handle;

    // Verbose Debugging mode
    bool is_debugging_;
};

inline const Shaders * Renderer::shaders() const {
  return &shaders_;
}

#endif

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

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "lib/shader/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Renderer {
  public:
    // Construct with verbose debugging mode
    Renderer(const bool &debug_flag = false);

    // Draws/Renders the passed in objects (with their models) to the scene
    //   @param Object * object, an object to render
    //   @param Camera * camera, to get the camera matrix and correctly position world
    //   @warn this function is not responsible for NULL PTRs
    void Render(const Object * object, const Camera * camera, bool renderToShadow = false) const;
    // Draws/Renders the passed in terrain to the scene
    //   @param Terrain * terrain, a terrain (cliffs/roads) to render
    //   @param Camera * camera, to get the camera matrix and correctly position world
    void Render(const Terrain * terrain, const Camera * camera, bool renderToShadow = false) const;
    // Render Coordinate Axis 
    //   @param Camera * camera, to get the camera matrix and correctly position world
    //   @warn requires VAO from EnableAxis
    void RenderAxis(const Camera * camera) const;
    // Enable x,y,z axis coordinates
    void EnableAxis(const GLuint &program_id);

    void RenderWater(const Water * water, const Camera * camera, const Skybox * Sky, bool renderToShadow = false) const;

    void RenderTerrain();

    void RenderSkybox(const Skybox * Sky, const Camera * camera) const;

    void SetFrame(const GLuint &program_id, unsigned int windowX, unsigned int windowY);

    inline void SetProjection(const glm::mat4 &projection);

    inline GLuint getFrameBuffer() const;

    inline GLuint getDepth() const;

  private:
    // The VAO Handle for the Axis Coordinates
    unsigned int coord_vao_handle;
    // The shader to use to render Axis Coordinates
    GLuint axis_program_id;

    // Verbose Debugging mode
    bool is_debugging_;

    GLuint frame_buffer_name_, depth_texture_, depth_rb_;
    glm::mat4 projection_ortho_;
    glm::mat4 projection_;
};
inline GLuint Renderer::getDepth() const
{
    return depth_texture_;
}
inline void Renderer::SetProjection(const glm::mat4 &projection) {
    projection_ =  projection;
}

inline GLuint Renderer::getFrameBuffer() const
{
    return frame_buffer_name_;
}
#endif

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
    void Render(const Object * object, const Camera * camera, const bool is_frame = false) const;
    // Draws/Renders the passed in terrain to the scene
    //   @param Terrain * terrain, a terrain (cliffs/roads) to render
    //   @param Camera * camera, to get the camera matrix and correctly position world
    void Render(const Terrain * terrain, const Camera * camera, const bool is_frame = false) const;
    // Render Coordinate Axis 
    //   @param Camera * camera, to get the camera matrix and correctly position world
    //   @warn requires VAO from EnableAxis
    void RenderAxis(const Camera * camera) const;
    // Enable x,y,z axis coordinates
    void EnableAxis(const GLuint program_id);

    void RenderWater(const Water * water, const Object * object, const Camera * camera, const Skybox * Sky) const;

    void RenderTerrain();

    void RenderSkybox(const Skybox * Sky, const Camera * camera) const;

    void SetFrame(const GLuint &program_id);
    GLuint frame_buffer_name_;
    GLuint depth_texture_;

  private:
    // The VAO Handle for the Axis Coordinates
    unsigned int coord_vao_handle;
    // The shader to use to render Axis Coordinates
    GLuint axis_program_id;

    // Verbose Debugging mode
    bool is_debugging_;
};

#endif

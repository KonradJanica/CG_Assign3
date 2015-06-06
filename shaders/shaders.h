#ifndef ASSIGN3_SHADERS_H_
#define ASSIGN3_SHADERS_H_

#include <cassert>
#include <string>
#include <cstdio>
#include <GL/glew.h>
#include "shader_compiler/shader.hpp"

// A Shader
//   Holds an ID and all possible handles of a shader
//   In debugging mode prints uniforms not found
//   @warn Uniforms not found will be -1
struct Shader {
  const GLuint Id;

  // Matrices
  const GLint          mvpHandle;
  const GLint           mvHandle;
  const GLint         normHandle;
  const GLint       texMapHandle;
  const GLint    shadowMapHandle;
  const GLint depthBiasMvpHandle;
  const GLint     depthMvpHandle;
  // Lighting
  const GLint   mtlAmbientHandle;
  const GLint   mtlDiffuseHandle;
  const GLint  mtlSpecularHandle;
  const GLint    shininessHandle;
  // Water
  const GLint       camPosHandle;
  // Skybox
  const GLint         cubeHandle;
  // Projection
  const GLint         projHandle;

  // Try to load all uniform handles
  //   Will print to stderr when handles are not found
  //     in debugging mode
  Shader(const std::string &vert_path, const std::string &frag_path, const bool is_debug) :
    Id(LoadShaders(vert_path.c_str(), frag_path.c_str())),
    mvpHandle(          glGetUniformLocation(Id, "mvp_matrix")),
    mvHandle(           glGetUniformLocation(Id, "modelview_matrix")),
    normHandle(         glGetUniformLocation(Id, "normal_matrix")),
    texMapHandle(       glGetUniformLocation(Id, "texMap")),
    shadowMapHandle(    glGetUniformLocation(Id, "shadowMap")),
    depthBiasMvpHandle( glGetUniformLocation(Id, "depth_bias_mvp_matrix")),
    depthMvpHandle(     glGetUniformLocation(Id, "depth_mvp_matrix")),
    // Lighting
    mtlAmbientHandle(   glGetUniformLocation(Id, "mtl_ambient")),
    mtlDiffuseHandle(   glGetUniformLocation(Id, "mtl_diffuse")),
    mtlSpecularHandle(  glGetUniformLocation(Id, "mtl_specular")),
    shininessHandle(    glGetUniformLocation(Id, "shininess")),
    // Water
    camPosHandle(       glGetUniformLocation(Id, "cameraPos")),
    // Skybox
    cubeHandle(         glGetUniformLocation(Id, "cubeMap")),
    // Projection
    projHandle(         glGetUniformLocation(Id, "modelview_matrix"))
  {
    if (is_debug) {
      const std::string file_string = vert_path.substr(vert_path.find_last_of('/') + 1);
      const char *      file        = file_string.c_str();
      if (mvpHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - mvpHandle\n", file);
      if (mvHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - mvHandle\n", file);
      if (normHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - normHandle\n", file);
      if (texMapHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - texMapHandle\n", file);
      if (shadowMapHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - shadowMapHandle\n", file);
      if (depthBiasMvpHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - depthBiasMvpHandle\n", file);
      if (depthMvpHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - depthMvpHandle\n", file);
      if (mtlAmbientHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - mtlAmbientHandle\n", file);
      if (mtlDiffuseHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - mtlDiffuseHandle\n", file);
      if (mtlSpecularHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - mtlSpecularHandle\n", file);
      if (shininessHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - shininessHandle\n", file);
      if (camPosHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - camPosHandle\n", file);
      if (cubeHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - cubeHandle\n", file);
      if (projHandle == -1)
        fprintf(stderr, "%s - Could not find uniform variables - projHandle\n", file);
      fprintf(stderr, "\n"); // Make spacing
    }
  }
};

// A Shaders class
//   Holds all shaders required for the program
//   @warn only one instance should ever be created
struct Shaders {
  const Shader  * AxisDebug; //only created in debug mode
  const Shader LightMappedGeneric;
  const Shader WaterGeneric;
  const Shader SkyboxGeneric;
  const Shader RainGeneric;
  const Shader DepthBuffer;

  // Load in all the shaders
  //   @param is_debug, true = load axis shader
  Shaders(const bool is_debug = false) :
    AxisDebug(is_debug ? new Shader("shaders/coord.vert", "shaders/coord.frag", is_debug) : 0 ),
    LightMappedGeneric( Shader("shaders/shaded.vert", "shaders/shaded.frag", is_debug)),
    WaterGeneric(       Shader("shaders/water.vert", "shaders/water.frag", is_debug)),
    SkyboxGeneric(      Shader("shaders/sky.vert", "shaders/sky.frag", is_debug)),
    RainGeneric(        Shader("shaders/rain.vert", "shaders/rain.frag", is_debug)),
    DepthBuffer(        Shader("shaders/depthbuffer.vert", "shaders/depthbuffer.frag", is_debug))
    {
      if (is_debug)
        assert(AxisDebug->Id       && "Axis Shader failed to load");
      assert(LightMappedGeneric.Id && "LightMappedGeneric Shader failed to load");
      assert(WaterGeneric.Id       && "WaterGeneric Shader failed to load");
      assert(SkyboxGeneric.Id      && "SkyboxGeneric Shader failed to load");
      assert(RainGeneric.Id        && "RainGeneric Shader failed to load");
      assert(DepthBuffer.Id        && "DepthBuffer Shader failed to load");
    }
};

#endif

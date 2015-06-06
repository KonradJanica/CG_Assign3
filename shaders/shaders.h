#ifndef ASSIGN3_SHADERS_H_
#define ASSIGN3_SHADERS_H_

#include <cassert>
#include "shader_compiler/shader.hpp"

// A Shaders class
//   Holds all shaders required for the program
//   @warn only one instane should ever be created
struct Shaders {
  const GLuint AxisDebug;
  const GLuint LightMappedGeneric;
  const GLuint WaterGeneric;
  const GLuint SkyboxGeneric;
  const GLuint RainGeneric;
  const GLuint DepthBuffer;

  // Load in all the shaders
  //   @param is_debug, true = load axis shader
  Shaders(const bool is_debug = false) :
    AxisDebug(is_debug ? LoadShaders("shaders/coord.vert", "shaders/coord.frag") : -1),
    LightMappedGeneric(LoadShaders("shaders/shaded.vert", "shaders/shaded.frag")),
    WaterGeneric(LoadShaders("shaders/water.vert", "shaders/water.frag")),
    SkyboxGeneric(LoadShaders("shaders/sky.vert", "shaders/sky.frag")),
    RainGeneric(LoadShaders("shaders/rain.vert", "shaders/rain.frag")),
    DepthBuffer(LoadShaders("shaders/depthbuffer.vert", "shaders/depthbuffer.frag"))
    {
      assert(AxisDebug && "Axis Shader failed to load");
      assert(LightMappedGeneric && "LightMappedGeneric Shader failed to load");
      assert(WaterGeneric && "WaterGeneric Shader failed to load");
      assert(SkyboxGeneric && "SkyboxGeneric Shader failed to load");
      assert(RainGeneric && "RainGeneric Shader failed to load");
      assert(DepthBuffer && "DepthBuffer Shader failed to load");
    }
};

#endif

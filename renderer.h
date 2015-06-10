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
#include "sun.h"
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

// The FBO that holds the depth texture
//   Used for shadow mapping
struct FrameBufferObject {
  // Size of texture
  const unsigned int textureX;
  const unsigned int textureY;

  GLuint FrameBufferShadows;
  GLuint DepthTexture;

  FrameBufferObject() :
  // Size of texture
  textureX(1024), textureY(1024) {

  glActiveTexture(GL_TEXTURE20);
  // generate namespace for the frame buffer 
  glGenFramebuffers(1, &FrameBufferShadows);
  //switch to our fbo so we can bind stuff to it
  glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferShadows);

  // and depthbuffer
  glGenTextures(1, &DepthTexture);
  // create the depth texture and attach it to the frame buffer.
  glBindTexture(GL_TEXTURE_2D, DepthTexture);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, textureX, textureY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  // glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, windowX, windowY, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // Set "renderedTexture" as our depth attachement
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

  // Instruct openGL that we won't bind a color texture with the currently binded FBO
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // Always check that our framebuffer is ok
  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      exit(-1);
  }

  // Unbind buffer
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  }
};

// The Class used for rendering
//   All DrawElement etc. openGL calls are made through this class
//   Everything should be const to maximize read only performance
class Renderer {
  public:
    // Construct with verbose debugging mode option
    Renderer(const bool debug_flag = false);

    // Draws/Renders the passed in objects (with their models) to the scene
    //   @param Object * object, an object to render
    //   @warn this function is not responsible for NULL PTRs
    //   @warn uses camera pointer for view matrix
    void Render(const Object * object, const Camera &camera, const Sun &sun) const;
    // TODO
    void RenderDepthBuffer(const Object * car, const Sun &sun) const;
    // Draws/Renders the passed in terrain to the scene
    //   @param Terrain * terrain, a terrain (cliffs/roads) to render
    //   @warn uses camera pointer for view matrix
    void Render(const Terrain * terrain, const Camera &camera, const Sun &sun) const;
    // TODO
    void RenderDepthBuffer(const Terrain * terrain, const Sun &sun) const;
    // Render Coordinate Axis 
    //   Only renders in debugging mode
    //   @warn requires VAO from EnableAxis
    void RenderAxis(const Camera &camera) const;
    // Enable x,y,z axis coordinates VAO
    //   @return a VAO to use for the Axis
    //   @warn should only be called once, duplicate calls are irrelevant
    GLuint EnableAxis() const;

    void RenderWater(const Water * water, const Object * object, const Skybox * Sky, const Camera &camera) const;

    void RenderSkybox(const Skybox * Sky, const Camera &camera) const;

    // Accessor for a shaders pointer
    inline const Shaders * shaders() const;
    // Accessor for a fbo pointer
    inline const FrameBufferObject * fbo() const;

  private:
    // The FBO to hold the shadow map buffer and it's depth texture
    const FrameBufferObject fbo_;

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

// Accessor for a fbo pointer
inline const FrameBufferObject * Renderer::fbo() const {
  return &fbo_;
}

#endif

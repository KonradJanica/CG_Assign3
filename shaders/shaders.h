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


// An iterator for the Shaders struct
//   The depthbuffer shader is at iter.end() but should be
//   hence is excluded from std begin->end looping
//   Helpful for updating all projections
//   @warn iterating past end() (or before begin) is undefined
//         behaviour and will eventually overflow to beginning
//   @warn hard coded
struct ShadersProjectionIterator {
  typedef ShadersProjectionIterator const_iterator;
  // BASE CONSTRUCTOR
  ShadersProjectionIterator(const Shaders * shaders) :
    shaders_(shaders) {};

  // ITERATOR OVERLOADED CONSTRUCTOR
  ShadersProjectionIterator(const Shaders * shaders, const Shader * shader, const char index) :
    shaders_(shaders), shader_iter_(shader), index_(index) {};

  // ITERATORS
  // begin(), An iterator referring to AxisDebug or LightMappedGeneric, i.e. the first element
  //          depends on whether debugging is enabled or not.
  //   @warn undefined behaviour when iterating past end (or before begin)
  const_iterator begin() const {
    const_iterator temp(
        this->shaders_,
        shaders_->AxisDebug ? shaders_->AxisDebug : &(shaders_->LightMappedGeneric),
        shaders_->AxisDebug ? 0 : 1);
    return temp;
  }
  // end(), An iterator referring to DepthBuffer, i.e. past-the-end (relevant-to-proj) element
  //   @warn undefined behaviour when iterating past end (or before begin)
  //   @warn will return axis debug if iterated past begin
  const_iterator end()   const {
    const_iterator temp(
        this->shaders_,
        &(shaders_->DepthBuffer),
        4);
    return temp;
  }
  // Dereference
  const Shader * operator * () { return shader_iter_; };
  const Shader * operator -> () { return (operator * ()); };
  // Mutators
  const_iterator &operator ++ () {
    switch (index_) {
      case 0:
        shader_iter_ = &shaders_->LightMappedGeneric;
        break;
      case 1:
        shader_iter_ = &shaders_->WaterGeneric;
        break;
      case 2:
        shader_iter_ = &shaders_->SkyboxGeneric;
        break;
      case 3:
        shader_iter_ = &shaders_->RainGeneric;
        break;
      case 4:
        shader_iter_ = &shaders_->DepthBuffer; // End of iterator
        break;
    }
    ++index_;
    return *this;
  };

  const_iterator operator ++ (int) {
    const_iterator temp(*this);
    ++(*this);
    return temp;
  };

  const_iterator &operator -- () {
    switch (index_) {
      --index_;
      case 0:
      shader_iter_ = shaders_->AxisDebug;
      case 1:
      shader_iter_ = &shaders_->LightMappedGeneric;
      break;
      case 2:
      shader_iter_ = &shaders_->WaterGeneric;
      break;
      case 3:
      shader_iter_ = &shaders_->SkyboxGeneric;
      break;
      case 4:
      shader_iter_ = &shaders_->RainGeneric;
      break;
      case 5:
      shader_iter_ = &shaders_->DepthBuffer; // End of iterator
      break;
    }
    return *this;
  };

  const_iterator operator -- (int) {
    const_iterator temp(*this);
    --(*this);
    return temp;
  };

  bool operator == (const const_iterator &other) const {
    return index_ == other.index_ &&
      shader_iter_ == other.shader_iter_ &&
      shaders_ == other.shaders_;
  }
  bool operator != (const const_iterator &other) const {
    return index_ != other.index_ &&
      shader_iter_ != other.shader_iter_ &&
      shaders_ != other.shaders_;
  }

  private:
  // The shaders object for next pointer
  const Shaders * shaders_;
  // The current shader object pointed to
  const Shader * shader_iter_;
  // The current index of the iterator
  //   Corresponds to the order of members
  //   in Shaders struct
  char index_;
};

#endif

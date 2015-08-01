/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * light_controller.h, definition of phong lighting models in openGL
 * 
 * This file is a definition of phong lighting models in openGL
 * including spotlights, point lights and directional lights
 *
 * 
 */

#ifndef ASSIGN3_LIGHT_CONTROLLER_H_
#define ASSIGN3_LIGHT_CONTROLLER_H_

#include "utils/includes.h"

// The light structures help the LightController class deal with light input

struct BaseLight 
{
  glm::vec3 AmbientIntensity;
  glm::vec3 DiffuseIntensity;
  glm::vec3 SpecularIntensity;

  BaseLight()
  {
    AmbientIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
    DiffuseIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
    SpecularIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
  }
};

struct DirectionalLight : BaseLight
{
  glm::vec3 Direction;

  DirectionalLight()
  {
    Direction = glm::vec3(0.0f, 0.0f, 0.0f);
  }
};

struct PointLight : BaseLight
{
  glm::vec3 Position;

  struct 
  {
    float Constant;
    float Linear;
    float Exp;
  } Attenuation;

  PointLight()
  {
    Position = glm::vec3(0.0f, 0.0f, 0.0f);
    Attenuation.Constant = 1.0f;
    Attenuation.Linear = 0.0f;
    Attenuation.Exp = 0.0f;
  }
};

struct SpotLight : PointLight
{
  glm::vec3 Direction;
  float CosineCutoff;

  SpotLight()
  {
    Direction = glm::vec3(0.0f, 0.0f, 0.0f);
    CosineCutoff = 0.0f;
  }
};

// Class to take care of sending light data into shader program

class LightController
{
public:
  // Sets light properties
  void SetDirectionalLight(GLuint program_id, const DirectionalLight& light);
  void SetPointLights(GLuint program_id, unsigned int numLights, const PointLight* lights);
  void SetSpotLights(GLuint program_id, unsigned int numLights, const SpotLight* lights);
};

#endif

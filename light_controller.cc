/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * light_controller.cc,  implementation of phong lighting models in openGL
 * 
 * This file is a simple implementation of phong lighting models in openGL
 * including spotlights, point lights and directional lights
 *
 * 
 */

#include "light_controller.h"

void LightController::SetDirectionalLight(GLuint program_id, const DirectionalLight& light)
{
  glUseProgram(program_id);

  int dirAmbientHandle = glGetUniformLocation(program_id, "gDirectionalLight.Base.AmbientIntensity");
  int dirDiffuseHandle = glGetUniformLocation(program_id, "gDirectionalLight.Base.DiffuseIntensity");
  int dirSpecularHandle = glGetUniformLocation(program_id, "gDirectionalLight.Base.SpecularIntensity");
  int dirDirectionHandle = glGetUniformLocation(program_id, "gDirectionalLight.Direction");

  if (dirAmbientHandle == -1 ||
      dirDiffuseHandle == -1 ||
      dirSpecularHandle == -1 ||
      dirDirectionHandle == -1) 
  {
    fprintf(stderr,"Could not find uniforms required for a directional light In: Light_Controller - SetDirectionalLight\n This may cause unexpected behaviour in the program\n");
  }


  glUniform3fv(dirAmbientHandle, 1, glm::value_ptr(light.AmbientIntensity));
  glUniform3fv(dirDiffuseHandle, 1, glm::value_ptr(light.DiffuseIntensity));
  glUniform3fv(dirSpecularHandle, 1, glm::value_ptr(light.SpecularIntensity));
  glUniform3fv(dirDirectionHandle, 1, glm::value_ptr(light.Direction));
}

void LightController::SetPointLights(GLuint program_id, unsigned int numLights, const PointLight* lights)
{
  glUseProgram(program_id);
  const unsigned int MAX_LIGHTS = 10;
  assert(numLights < MAX_LIGHTS && "Exceeded maximum allowable amount of point lights\n");
  
  int numPointLightsHandle = glGetUniformLocation(program_id, "gNumPointLights");
  if (numPointLightsHandle == -1) {
    fprintf(stderr,"Could not find uniform: 'gNumPointLights' Light_Controller - SetPointLights\n This may cause unexpected behaviour in the program\n");
  }

  glUniform1i(numPointLightsHandle, numLights);

  for (unsigned int i = 0; i < numLights; i++)
  {
    char uniformName[256];
    memset(uniformName, 0, sizeof(uniformName));

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.AmbientIntensity", i);
    int pointAmbientHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.DiffuseIntensity", i);
    int pointDiffuseHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.SpecularIntensity", i);
    int pointSpecularHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Position", i);
    int pointPositionHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Constant", i);
    int pointAttenConstantHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Linear", i);
    int pointAttenLinearHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Exp", i);
    int pointAttenExpHandle = glGetUniformLocation(program_id, uniformName);

    if (pointAmbientHandle == -1 || pointDiffuseHandle == -1 || pointSpecularHandle == -1 ||
        pointPositionHandle == -1 || pointAttenConstantHandle == -1 || 
        pointAttenLinearHandle == -1 || pointAttenExpHandle == -1) 
    {
      fprintf(stderr,"Could not find uniforms required for a point light In: Light_Controller - SetPointLights\n This may cause unexpected behaviour in the program\n");
    }

    glUniform3fv(pointAmbientHandle, 1, glm::value_ptr(lights[i].AmbientIntensity));
    glUniform3fv(pointDiffuseHandle, 1, glm::value_ptr(lights[i].DiffuseIntensity));
    glUniform3fv(pointSpecularHandle, 1, glm::value_ptr(lights[i].SpecularIntensity));
    glUniform3fv(pointPositionHandle, 1, glm::value_ptr(lights[i].Position));
    glUniform1f(pointAttenConstantHandle, lights[i].Attenuation.Constant);
    glUniform1f(pointAttenLinearHandle, lights[i].Attenuation.Linear);
    glUniform1f(pointAttenExpHandle, lights[i].Attenuation.Exp);
  }
}

void LightController::SetSpotLights(GLuint program_id, unsigned int numLights, const SpotLight* lights)
{
  glUseProgram(program_id);
  const unsigned int MAX_LIGHTS = 10;
  assert(numLights < MAX_LIGHTS && "Exceeded maximum allowable amount of spot lights\n");

  int numSpotLightsHandle = glGetUniformLocation(program_id, "gNumSpotLights");
  if (numSpotLightsHandle == -1) {
     fprintf(stderr,"Could not find uniform: 'gNumSpotLights' Light_Controller - SetSpotLights\n This may cause unexpected behaviour in the program\n");
    exit(0);
  }

  glUniform1i(numSpotLightsHandle, numLights);

  for (unsigned int i = 0; i < numLights; i++)
  {
    char uniformName[256];
    memset(uniformName, 0, sizeof(uniformName));

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
    int spotAmbientHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
    int spotDiffuseHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.SpecularIntensity", i);
    int spotSpecularHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Position", i);
    int spotPositionHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Constant", i);
    int spotAttenConstantHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Linear", i);
    int spotAttenLinearHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Exp", i);
    int spotAttenExpHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Direction", i);
    int spotDirectionHandle = glGetUniformLocation(program_id, uniformName);

    snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].CosineCutoff", i);
    int spotCosineHandle = glGetUniformLocation(program_id, uniformName);

    if (spotAmbientHandle == -1 || spotDiffuseHandle == -1 || spotSpecularHandle == -1 ||
        spotPositionHandle == -1 || spotAttenConstantHandle == -1 ||
        spotAttenLinearHandle == -1 || spotAttenExpHandle == -1 ||
        spotDirectionHandle == -1 || spotCosineHandle == -1)
    {
      fprintf(stderr,"Could not find uniforms required for a spoy light In: Light_Controller - SetSpotLights\n This may cause unexpected behaviour in the program\n");
    }

    glUniform3fv(spotAmbientHandle, 1, glm::value_ptr(lights[i].AmbientIntensity));
    glUniform3fv(spotDiffuseHandle, 1, glm::value_ptr(lights[i].DiffuseIntensity));
    glUniform3fv(spotSpecularHandle, 1, glm::value_ptr(lights[i].SpecularIntensity));
    glUniform3fv(spotPositionHandle, 1, glm::value_ptr(lights[i].Position));
    glUniform3fv(spotDirectionHandle, 1, glm::value_ptr(lights[i].Direction));
    glUniform1f(spotCosineHandle, lights[i].CosineCutoff);
    glUniform1f(spotAttenConstantHandle, lights[i].Attenuation.Constant);
    glUniform1f(spotAttenLinearHandle, lights[i].Attenuation.Linear);
    glUniform1f(spotAttenExpHandle, lights[i].Attenuation.Exp);
  }
}

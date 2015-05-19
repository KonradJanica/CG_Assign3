#include "light_controller.h"

LightController::LightController()
{

}

bool LightController::Init(GLuint program_id)
{
	program_id_ = program_id;

	glUseProgram(program_id_);

	num_point_lights_location_ = glGetUniformLocation(program_id_, "gNumPointLights");
	num_spot_lights_location_ = glGetUniformLocation(program_id_, "gNumSpotLights");
	directional_light_location_.AmbientIntensity = glGetUniformLocation(program_id_, "gDirectionalLight.Base.AmbientIntensity");
	directional_light_location_.DiffuseIntensity = glGetUniformLocation(program_id_, "gDirectionalLight.Base.DiffuseIntensity");
	directional_light_location_.SpecularIntensity = glGetUniformLocation(program_id_, "gDirectionalLight.Base.SpecularIntensity");
	directional_light_location_.Direction = glGetUniformLocation(program_id_, "gDirectionalLight.Direction");

	if (num_point_lights_location_ == -1 ||
		num_spot_lights_location_ == -1 ||
		directional_light_location_.AmbientIntensity == -1 ||
		directional_light_location_.DiffuseIntensity == -1 ||
		directional_light_location_.SpecularIntensity == -1 ||
		directional_light_location_.Direction == -1) 
	{
		return false;
	}

	return true;
}

void LightController::SetDirectionalLight(const DirectionalLight& light)
{
	// Send to shader
	glUseProgram(program_id_);
	glUniform3fv(directional_light_location_.AmbientIntensity, 1, glm::value_ptr(light.AmbientIntensity));
	glUniform3fv(directional_light_location_.DiffuseIntensity, 1, glm::value_ptr(light.DiffuseIntensity));
	glUniform3fv(directional_light_location_.SpecularIntensity, 1, glm::value_ptr(light.SpecularIntensity));
	glUniform3fv(directional_light_location_.Direction, 1, glm::value_ptr(light.Direction));
}

void LightController::SetPointLights(unsigned int numLights, const PointLight* lights)
{
	const unsigned int MAX_LIGHTS = 10;
	assert(numLights < MAX_LIGHTS && "Exceeded maximum allowable amount of point lights\n");

	glUseProgram(program_id_);

	glUniform1i(num_point_lights_location_, numLights);

	point_light_locations_.resize(numLights);

	for (unsigned int i = 0; i < numLights; i++)
	{
		char uniformName[256];
		memset(uniformName, 0, sizeof(uniformName));

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.AmbientIntensity", i);
		point_light_locations_[i].AmbientIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.DiffuseIntensity", i);
		point_light_locations_[i].DiffuseIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Base.SpecularIntensity", i);
		point_light_locations_[i].SpecularIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Position", i);
		point_light_locations_[i].Position = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Constant", i);
		point_light_locations_[i].Attenuation.Constant = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Linear", i);
		point_light_locations_[i].Attenuation.Linear = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gPointLights[%d].Atten.Exp", i);
		point_light_locations_[i].Attenuation.Exp = glGetUniformLocation(program_id_, uniformName);

		glUniform3fv(point_light_locations_[i].AmbientIntensity, 1, glm::value_ptr(lights[i].AmbientIntensity));
		glUniform3fv(point_light_locations_[i].DiffuseIntensity, 1, glm::value_ptr(lights[i].DiffuseIntensity));
		glUniform3fv(point_light_locations_[i].SpecularIntensity, 1, glm::value_ptr(lights[i].SpecularIntensity));
		glUniform3fv(point_light_locations_[i].Position, 1, glm::value_ptr(lights[i].Position));
		glUniform1f(point_light_locations_[i].Attenuation.Constant, lights[i].Attenuation.Constant);
		glUniform1f(point_light_locations_[i].Attenuation.Linear, lights[i].Attenuation.Linear);
		glUniform1f(point_light_locations_[i].Attenuation.Exp, lights[i].Attenuation.Exp);
	}
}

void LightController::SetSpotLights(unsigned int numLights, const SpotLight* lights)
{
	const unsigned int MAX_LIGHTS = 10;
	assert(numLights < MAX_LIGHTS && "Exceeded maximum allowable amount of spot lights\n");

	glUseProgram(program_id_);

	glUniform1i(num_spot_lights_location_, numLights);

	spot_light_locations_.resize(numLights);

	for (unsigned int i = 0; i < numLights; i++)
	{
		char uniformName[256];
		memset(uniformName, 0, sizeof(uniformName));

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
		spot_light_locations_[i].AmbientIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
		spot_light_locations_[i].DiffuseIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Base.SpecularIntensity", i);
		spot_light_locations_[i].SpecularIntensity = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Position", i);
		spot_light_locations_[i].Position = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Constant", i);
		spot_light_locations_[i].Attenuation.Constant = glGetUniformLocation(program_id_, uniformName);
		
		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Linear", i);
		spot_light_locations_[i].Attenuation.Linear = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Base.Atten.Exp", i);
		spot_light_locations_[i].Attenuation.Exp = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].Direction", i);
		spot_light_locations_[i].Direction = glGetUniformLocation(program_id_, uniformName);

		snprintf(uniformName, sizeof(uniformName), "gSpotLights[%d].CosineCutoff", i);
		spot_light_locations_[i].CosineCutoff = glGetUniformLocation(program_id_, uniformName);

		glUniform3fv(spot_light_locations_[i].AmbientIntensity, 1, glm::value_ptr(lights[i].AmbientIntensity));
		glUniform3fv(spot_light_locations_[i].DiffuseIntensity, 1, glm::value_ptr(lights[i].DiffuseIntensity));
		glUniform3fv(spot_light_locations_[i].SpecularIntensity, 1, glm::value_ptr(lights[i].SpecularIntensity));
		glUniform3fv(spot_light_locations_[i].Position, 1, glm::value_ptr(lights[i].Position));
		glUniform3fv(spot_light_locations_[i].Direction, 1, glm::value_ptr(lights[i].Direction));
		glUniform1f(spot_light_locations_[i].CosineCutoff, lights[i].CosineCutoff);
		glUniform1f(spot_light_locations_[i].Attenuation.Constant, lights[i].Attenuation.Constant);
		glUniform1f(spot_light_locations_[i].Attenuation.Linear, lights[i].Attenuation.Linear);
		glUniform1f(spot_light_locations_[i].Attenuation.Exp, lights[i].Attenuation.Exp);
	}
}

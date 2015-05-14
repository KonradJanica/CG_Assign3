#include "light_controller.h"

LightController::LightController()
{

}

bool LightController::Init(GLuint program_id)
{
	program_id_ = program_id;

}

void LightController::SetDirectionalLight(const DirectionalLight& light)
{
	glUniform3fv(directional_light_location_.AmbientIntensity, 1, glm::value_ptr(light.AmbientIntensity));
	glUniform3fv(directional_light_location_.DiffuseIntensity, 1, glm::value_ptr(light.DiffuseIntensity));
	glUniform3fv(directional_light_location_.SpecularIntensity, 1, glm::value_ptr(light.SpecularIntensity));
	glUniform3fv(directional_light_location_.Direction, 1, glm::value_ptr(light.Direction));
}

void LightController::SetPointLights(unsigned int numLights, const PointLight* lights)
{
	glUniform1i(num_point_lights_location_, numLights);

	point_light_locations_.resize(numLights);

	for (unsigned int i = 0; i < numLights; i++)
	{
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
	glUniform1i(num_spot_lights_location_, numLights);

	spot_light_locations_.resize(numLights);

	for (unsigned int i = 0; i < numLights; i++)
	{
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
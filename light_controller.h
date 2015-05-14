#ifndef ASSIGN3_LIGHT_CONTROLLER_H_
#define ASSIGN3_LIGHT_CONTROLLER_H_

#include "glm/glm.hpp"

struct BaseLight {
	glm::vec3 Colour;
	float AmbientIntensity;
	float DiffuseIntensity;

	BaseLight()
	{
		Colour = glm::vec3(0.0f, 0.0f, 0.0f);
		AmbientIntensity = 0.0f;
		DiffuseIntensity = 0.0f;
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
		Position = glm::vec3();
	}


};


class LightController {
public:
	bool Init();
	void SetDirectionalLight(const )

private:
	GLuint program_id_;

};

#endif
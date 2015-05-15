#ifndef ASSIGN3_LIGHT_CONTROLLER_H_
#define ASSIGN3_LIGHT_CONTROLLER_H_

#include <cassert>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

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
	LightController();

	// Gets uniform locations 
	bool Init(GLuint program_id);

	// Sets light properties
	void SetDirectionalLight(const DirectionalLight& light);
	void SetPointLights(unsigned int numLights, const PointLight* lights);
	void SetSpotLights(unsigned int numLights, const SpotLight* lights);


private:
	GLuint program_id_;

	GLint num_point_lights_location_;
	GLint num_spot_lights_location_;

	// Uniform locations for directional light
	struct DirectionalLightLocation 
	{
		GLint AmbientIntensity;
		GLint DiffuseIntensity;
		GLint SpecularIntensity;
		GLint Direction;
	} directional_light_location_;

	struct PointLightLocation
	{
		GLint AmbientIntensity;
		GLint DiffuseIntensity;
		GLint SpecularIntensity;
		GLint Position;
		struct 
		{
			GLint Constant;
			GLint Linear;
			GLint Exp;
		} Attenuation;
	};

	// Uniform locations for pointlights
	std::vector<PointLightLocation> point_light_locations_;

	struct SpotLightLocation
	{
		GLint AmbientIntensity;
		GLint DiffuseIntensity;
		GLint SpecularIntensity;
		GLint Position;
		GLint Direction;
		GLint CosineCutoff;
		struct {
			GLint Constant;
			GLint Linear;
			GLint Exp;
		} Attenuation;
	};

	// Uniform locations for spotlights
	std::vector<SpotLightLocation> spot_light_locations_;
};

#endif

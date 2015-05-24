#version 130



struct BaseLight
{
  vec3 AmbientIntensity;
  vec3 DiffuseIntensity;
  vec3 SpecularIntensity;
};

struct Attenuation
{
  float Constant;
  float Linear;
  float Exp;
};

struct DirectionalLight
{
  BaseLight Base;
  vec3 Direction;
};

struct PointLight
{
  BaseLight Base;
  vec3 Position;
  Attenuation Atten;
};

struct SpotLight
{
  PointLight Base;
  vec3 Direction;
  float CosineCutoff;
};

const int MAX_POINT_LIGHTS = 10;
const int MAX_SPOT_LIGHTS = 10;

// Light properties
uniform int gNumPointLights;
uniform int gNumSpotLights;
uniform DirectionalLight gDirectionalLight;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];

// Material properties
uniform float mtl_ambient;
uniform vec3 mtl_diffuse;
uniform vec3 mtl_specular;
uniform float shininess;

uniform samplerCube skybox;

uniform vec3 cameraPos;

in vec4 a_vertex_mv;
in vec4 colour;
in vec3 a_normal_mv;
out vec4 fragColour;



// lightDirection is the vector from the light source to the target point
vec4 phongLight(in BaseLight light, in vec3 lightDirection, in vec3 normal)
{ 
  vec4 ambientColour = vec4(light.AmbientIntensity * mtl_ambient, 1.0);
  vec4 diffuseColour = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 specularColour = vec4(0.0, 0.0, 0.0, 0.0);

  float diffuseFactor = dot(-lightDirection, normal);

  if (diffuseFactor > 0) 
  {
    diffuseColour = vec4(diffuseFactor * light.DiffuseIntensity * mtl_diffuse, 1.0);

    vec3 reflectionVector = normalize(reflect(lightDirection, normal));
    float specularFactor = dot(reflectionVector, normal);

    if (specularFactor > 0)
    {
      specularColour = vec4(pow(specularFactor, shininess) * light.SpecularIntensity, 1.0); 
    }
  }

  return ambientColour + diffuseColour + specularColour;
}

vec4 calcDirectionalLight(in vec3 normal)
{
  return phongLight(gDirectionalLight.Base, normalize(gDirectionalLight.Direction), normal);
}

vec4 calcPointLight(in PointLight light, in vec4 position, in vec3 normal)
{
  vec3 lightDirection = position.xyz - light.Position;
  float distance = length(lightDirection);
  lightDirection = normalize(lightDirection);

  vec4 colour = phongLight(light.Base, lightDirection, normal);
  float attenuation = light.Atten.Constant +
                      light.Atten.Linear * distance +
                      light.Atten.Exp * distance * distance;

  return colour / attenuation;
}

vec4 calcSpotLight(in SpotLight light, in vec4 position, in vec3 normal)
{
  vec3 lightToPositionDirection = normalize(position.xyz - light.Base.Position);
  float spotFactor = dot(lightToPositionDirection, normalize(light.Direction));

  if (spotFactor > light.CosineCutoff)
  {
    return calcPointLight(light.Base, position, normal) * 
           (1.0 - (1.0 - spotFactor) * 1.0 / (1.0 - light.CosineCutoff));
  }
  else 
  {
    return vec4(0.0, 0.0, 0.0, 0.0);
  }
}

void main(void) {

  vec3 normal_mv = normalize(a_normal_mv);
  float ratio = 1.00 / 1.33;
  vec3 I = normalize(vec3(a_vertex_mv.x, a_vertex_mv.y, a_vertex_mv.z) - cameraPos);
  vec3 R = refract(I, normalize(normal_mv), ratio);

  vec4 litColour = calcDirectionalLight(normal_mv);
  for (int i = 0; i < gNumPointLights; i++)
  {
    litColour += calcPointLight(gPointLights[i], a_vertex_mv, normal_mv);
  }

  for (int i = 0; i < gNumSpotLights; i++)
  {
    litColour += calcSpotLight(gSpotLights[i], a_vertex_mv, normal_mv);
  }

  vec4 colour = litColour * vec4(1.0, 0.0, 1.0, 1.0);
  fragColour = colour;
  //colour = mix(colour,vec4(0.0, 0.0, 1.0, 1.0), 0.1);
  //colour.a = 1.0;

  //fragColour = texture(skybox, R) * litColour;

  //fragColour = vec4(1.0, 1.0, 1.0, 1.0);
}

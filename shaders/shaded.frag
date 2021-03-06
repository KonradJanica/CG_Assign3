#version 130

// 0 = No fog, 1 = Linear, 2 = Exponential, 3 = Exponentional (diff)
uniform int fog_mode;

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
// Bias for trimming shadow acne
const float BIAS = 0.001;

// Light properties
uniform int gNumPointLights;
uniform int gNumSpotLights;
uniform DirectionalLight gDirectionalLight;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];

// Material properties
uniform vec3 mtl_ambient;
uniform vec3 mtl_diffuse;
uniform vec3 mtl_specular;
uniform float shininess;
uniform float dissolve;

uniform int isBumped;
uniform float shadowIntensity;

uniform sampler2D texMap;
uniform sampler2D normMap;
uniform sampler2D mossMap;
uniform sampler2DShadow shadowMap;

in vec4 a_vertex_mv;
in vec3 a_normal_mv;
in vec2 a_tex_coord;
in vec4 a_shadow_coord;

out vec4 fragColour;

// @author - Mitch
// Use different fog calculations to determine how fog is
// visible
// @params  - fogCoord =  the abs Z distance of the vertex in eye space
//          - begin =     where fog begins to appear
//          - end =       where fog completely engulfs (make sure this value is quite large)
//          - density =   how dense the fog is (used in exponential eqn)

float fogFactor(vec4 fogCoord, float begin, float end, float density)
{
  float fogFac;
  float fogZ = abs(fogCoord.z);
  float fogY = abs(fogCoord.y);

  fogFac = exp(-pow(density*fogZ, 2.0)); 


  fogFac = clamp( fogFac, 0.0, 1.0 );
  return fogFac;
}

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

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

  // Cannot trust pipeline interpolation to generate normalized normals

  vec4 vertex_mv = a_vertex_mv;
  vec3 normal_mv = normalize(a_normal_mv);  
 
  if(isBumped > 0)
  {
    vec3 NN = texture(normMap, a_tex_coord.st).xyz; // normal map
    normal_mv  =  normal_mv +  normalize(2.0*NN.xyz-1.0);
  }


  vec4 litColour = calcDirectionalLight(normal_mv);

  for (int i = 0; i < gNumPointLights; i++)
  {
    litColour += calcPointLight(gPointLights[i], vertex_mv, normal_mv);
  }

  for (int i = 0; i < gNumSpotLights; i++)
  {
    litColour += calcSpotLight(gSpotLights[i], vertex_mv, normal_mv);
  }

	if(isBumped > 0)
  {
    litColour = mix(litColour, texture(mossMap, a_tex_coord), 0.2);
  }

  float visibility = texture(shadowMap, vec3(a_shadow_coord.xy, (a_shadow_coord.z-BIAS)/a_shadow_coord.w) );
  visibility = visibility / 2 + 0.5; //Fit range between [0,0.5]
  visibility *= shadowIntensity;
  litColour *= 1.6+(1-shadowIntensity);

  fragColour = mix(vec4(0.7,0.7,0.7,1.0), visibility * litColour * texture(texMap, a_tex_coord), fogFactor(vertex_mv,15.0,80.0,0.008));
  fragColour.a = dissolve;
}

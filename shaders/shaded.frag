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

uniform sampler2D texMap;

uniform sampler2D shadowMap;

in vec4 a_vertex_mv;
in vec3 a_normal_mv;
in vec2 a_tex_coord;
in vec4 shadow_coord;

uniform int isShadow;

out vec4 fragColour;
out float fragDepth;

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
  if (fog_mode == 0)
  {
    fogFac = 1.0;
  }
  else if(fog_mode == 1)
  {
    fogFac = (end - fogZ)/(end - begin);
  }
  else if(fog_mode == 2)
  {
    fogFac = exp(-density*fogZ); 
  }
  else if(fog_mode == 3)
  {
    fogFac = exp(-pow(density*fogZ, 2.0)); 
  }
  else if(fog_mode == 4) // N.B This is going to look pretty funky because model and view are not detached
  {
    // Code from http://in2gpu.com/2014/07/22/create-fog-shader/
    float be = (10.0 - fogY) * 0.004;
    float bi = (10.0 - fogY) * 0.001;
    float ext = exp(-fogZ * be);
    float insc = exp(-fogZ * bi);
    fogFac = ext + (1 - insc);
  }
  fogFac = clamp( fogFac, 0.0, 1.0 );
  return fogFac;
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
  if(isShadow == 1){
    fragDepth = gl_FragCoord.z;
  }
  else
  {
    // Calc Shadow Factor
    float bias = 0.005;
    float visibility = 1.0;
    if(texture(shadowMap, shadow_coord.xy).x < shadow_coord.z - bias)
    {
      visibility = 0.000;
    }
    // Cannot trust pipeline interpolation to generate normalized normals
    vec4 vertex_mv = a_vertex_mv;
    vec3 normal_mv = normalize(a_normal_mv); 
  
    vec4 litColour = calcDirectionalLight(normal_mv);
  
    for (int i = 0; i < gNumPointLights; i++)
    {
      litColour += calcPointLight(gPointLights[i], vertex_mv, normal_mv);
    }
  
    for (int i = 0; i < gNumSpotLights; i++)
    {
      litColour += calcSpotLight(gSpotLights[i], vertex_mv, normal_mv);
    }
  
    //fragColour = litColour * texture(texMap, a_tex_coord);
  
    fragColour = visibility * mix(vec4(0.7,0.7,0.7,1.0), litColour * texture(texMap, a_tex_coord), fogFactor(vertex_mv,15.0,80.0,0.01));
  }
}

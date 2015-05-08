/*********************************************************
  Version 1.000

  Code provided by Michael Hemsley and Anthony Dick
  for the course 
  COMP SCI 3014/7090 Computer Graphics
  School of Computer Science
  University of Adelaide

  Permission is granted for anyone to copy, use, modify, or distribute this
  program and accompanying programs and documents for any purpose, provided
  this copyright notice is retained and prominently displayed, along with
  a note saying that the original programs are available from the aforementioned 
  course web page. 

  The programs and documents are distributed without any warranty, express or
  implied.  As the programs were written for research purposes only, they have
  not been tested to the degree that would be advisable in any important
  application.  All use of these programs is entirely at the user's own risk.
 *********************************************************/

#version 130

// manyAttributes.vp
// This shader passes on colour values to be interpolated by the fixed functionality
// 

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

uniform vec4 light_pos;			// Light position in eye-space

uniform vec3 light_ambient;     // Light ambient RGBA values
uniform vec3 light_diffuse;     // Light diffuse RGBA values  
uniform vec3 light_specular;    // Light specular RGBA values
uniform int light_toggle;      // Toggles lighting on off - false = off

uniform vec3 mtl_ambient;  // Ambient surface colour
uniform vec3 mtl_diffuse;  // Diffuse surface colour
uniform vec3 mtl_specular; // Specular surface colour
uniform float shininess; // See below
// The shininess value is an exponent for the specular reflection
// It controls how spread out or sharp the highlight is
// It is the property of the surface material, not the light
// 4 - matte
// 16 - glossy/metallic
// 64 - plastic
// 128 - wet
// const float shininess = 32;

in vec3 a_vertex;
in vec2 a_texture;
in vec3 a_normal;
// in vec4 a_colour;

// This colour attribute will be sent out per-vertex and the fixed-functionality
// interpolates the colour (smooths the colour between) with connected vertices in the triangle
out vec2 text_coord;
out vec4 litColour;

// These control how quickly the light falls off based on distance
// Not used in this version of the shader.
const float constantAttn = 0.0;
const float linearAttn = 0.0;
const float quadraticAttn = 0.05;


// This function computes the Phong lit colour for a vertex
// Parameters are the vertex position and normal in eye space.
vec3 phongPointLight(in vec4 position, in vec3 norm)
{
  // s is the direction from the light to the vertex
  vec3 s = normalize(vec3(light_pos - position));

  // v is the direction from the eye to the vertex
  vec3 v = normalize(-position.xyz);

  // r is the direction of light reflected from the vertex
  vec3 r = reflect( -s, norm );

  vec3 ambient = light_ambient * mtl_ambient;

  // The diffuse component
  float sDotN = max( dot(s,norm), 0.0 );
  vec3 diffuse = light_diffuse * mtl_diffuse * sDotN;

  // Specular component
  vec3 spec = vec3(0.0);
  if ( sDotN > 0.0 )
    spec = light_specular * mtl_specular *
      pow( max( dot(r,v), 0.0 ), shininess );

  return ambient + diffuse + spec;
  //return diffuse + spec;
}

void main(void) {

  // Convert normal and position to eye coords
  vec3 eyeNorm = normalize( normal_matrix * a_normal);
  vec4 eyePos = modelview_matrix * vec4(a_vertex, 1.0);

if (light_toggle == 1) {
  litColour = vec4(phongPointLight(eyePos, eyeNorm), 1.0);
} else {
  //litColour = a_colour;
  litColour = vec4(1,1,1,1);;
}

  gl_Position = projection_matrix * eyePos;
  text_coord = a_texture;


}

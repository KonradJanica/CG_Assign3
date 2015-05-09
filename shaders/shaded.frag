#version 130

// Toggles lighting - 0 = off, 1 = on 
uniform int light_toggle;

// Light properties
uniform vec4 light_pos;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_specular;

// Material properties
uniform vec3 mtl_ambient;
uniform vec3 mtl_diffuse;
uniform vec3 mtl_specular;
uniform float shininess;

uniform sampler2D texMap;

in vec4 a_vertex_mv;
in vec3 a_normal_mv;
in vec2 a_tex_coord;

out vec4 fragColour;

vec3 phongLight(in vec4 position, in vec3 norm)
{
  // Direction from the light to the vertex
  vec3 s;
  if (light_pos.w == 0.0) {
    s = normalize(light_pos.xyz); // light_pos is a direction
  }
  else {
    s = normalize(vec3(light_pos - position)); // light_pos is a point
  }

  // Direction from the eye to the vertex
  vec3 v = normalize(-position.xyz);

  // Direction of light reflected from the vertex
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
}

void main(void) {

  // Cannot trust pipeline interpolation to generate normalized normals
  vec4 vertex_mv = a_vertex_mv;
  vec3 normal_mv = normalize(a_normal_mv); 

  vec4 litColour;

  if (light_toggle == 1) {
    litColour = vec4(phongLight(vertex_mv, normal_mv), 1.0);
  }
  else {
    litColour = vec4(1.0, 1.0, 1.0, 1.0);
  }

	fragColour = litColour * texture(texMap, a_tex_coord);
}

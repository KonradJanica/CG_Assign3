#version 130

uniform samplerCube cubeMap;

in vec4 a_vertex_mv;
in vec3 a_normal_mv;
in vec3 pos;

out vec4 fragColour;


void main(void) {

  // Cannot trust pipeline interpolation to generate normalized normals
  vec4 vertex_mv = a_vertex_mv;
  vec3 normal_mv = normalize(a_normal_mv); 

  vec4 litColour;
	fragColour = vec4(1.0, 1.0, 0.0, 1.0);
  
}

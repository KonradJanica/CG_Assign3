#version 130

uniform mat4 mvp_matrix;

in vec3 a_vertex;

out vec4 colour;
out vec3 texCoords;

void main(void) {
	
	texCoords = a_vertex;
	gl_Position = mvp_matrix * vec4(a_vertex, 1.0);

}

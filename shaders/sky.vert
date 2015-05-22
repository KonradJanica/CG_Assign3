#version 150

// manyAttributes.vp
// This shader passes on colour values to be interpolated by the fixed functionality
// 

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 a_vertex;

out vec4 colour;
out vec3 texCoords;

void main(void) {
	
	texCoords = a_vertex;
	gl_Position = projection_matrix * modelview_matrix * vec4(a_vertex, 1.0);

}

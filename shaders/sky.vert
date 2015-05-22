#version 150

// manyAttributes.vp
// This shader passes on colour values to be interpolated by the fixed functionality
// 

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;


in vec3 a_vertex;
in vec4 a_colour;	// The new, per-vertex attribute


out vec4 colour;
out vec3 texCoords;

void main(void) {
	
	colour = a_colour;	// We simply pass the colour along to the next stage
	texCoords = a_vertex;
	gl_Position = projection_matrix * modelview_matrix * vec4(a_vertex, 1.0);

}

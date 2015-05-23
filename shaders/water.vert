#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

uniform vec3 h;

in vec3 a_vertex;
out vec4 a_vertex_mv;


// Variables from http://jayconrod.com/posts/34/water-simulation-in-glsl


void main()
{


	a_vertex_mv = modelview_matrix * vec4(a_vertex, 1.0);
	// Apply full MVP transformation
	gl_Position = projection_matrix * a_vertex_mv;
}
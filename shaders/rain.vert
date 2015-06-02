#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 initial_vertices;
in vec3 displaced_vertices;
in vec4 colour;

uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;

out vec4 colour_f;

void main()
{
  colour_f = colour;

  vec3 vertexPosition_worldspace = 
		displaced_vertices
		+ CameraRight_worldspace * initial_vertices.x
		+ CameraUp_worldspace * initial_vertices.y;

  gl_Position = projection_matrix * modelview_matrix * vec4(vertexPosition_worldspace, 1.0);
}
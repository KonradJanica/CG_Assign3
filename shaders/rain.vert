#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 initial_vertices;
in vec3 displaced_vertices;
in vec4 colour;

out vec4 colour_f;

void main()
{
  colour_f = colour;

  gl_Position = projection_matrix * modelview_matrix * vec4(initial_vertices, 1.0);
}
#version 130

uniform mat4 mvp_matrix;

in vec3 initial_vertices;
in vec3 displaced_vertices;
in vec4 colour;

uniform vec3 cam_right;
uniform vec3 cam_up;

out vec4 colour_f;

void main()
{
  colour_f = colour;

  vec3 vertexpos_world = displaced_vertices + cam_right * initial_vertices.x + cam_up * initial_vertices.y;

  gl_Position = mvp_matrix * vec4(vertexpos_world, 1.0);
}

#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

in vec3 a_vertex;
in vec2 a_texture;
in vec3 a_normal;

out vec4 a_vertex_mv;
out vec3 a_normal_mv;
out vec2 a_tex_coord;

void main()
{
  // Pass pipeline the vertex position and normal in eye coordinates for light computation
  a_vertex_mv = modelview_matrix * vec4(a_vertex, 1.0);
  a_normal_mv = normalize(normal_matrix * a_normal);

  // Texture coordinates 
  a_tex_coord.x = a_texture.x;
  a_tex_coord.y = -a_texture.y;

  // Apply full MVP transformation
  gl_Position = projection_matrix * a_vertex_mv;
}
#version 130

uniform mat4 modelview_matrix;
uniform mat4 mvp_matrix;
uniform mat4 depth_bias_mvp_matrix;
uniform mat3 normal_matrix;

in vec3 a_vertex;
in vec2 a_texture;
in vec3 a_normal;

out vec4 a_vertex_mv;
out vec3 a_normal_mv;
out vec2 a_tex_coord;
out vec4 a_shadow_coord;

void main()
{
  // Pass pipeline the vertex position and normal in eye coordinates for light computation
  a_vertex_mv = modelview_matrix * vec4(a_vertex, 1.0);
  a_normal_mv = normalize(normal_matrix * a_normal);

  // Texture coordinates 
  a_tex_coord = a_texture;
  a_shadow_coord = depth_bias_mvp_matrix * vec4(a_vertex, 1.0);

  // Apply full MVP transformation
  gl_Position = mvp_matrix * vec4(a_vertex, 1.0);
}

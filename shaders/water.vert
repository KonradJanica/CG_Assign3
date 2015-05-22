#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

uniform float elapsed;

in vec3 b_vertex;
in vec2 a_texture;
in vec3 a_normal;

out vec4 a_vertex_mv;
out vec3 a_normal_mv;
out vec3 pos;


void main()
{

	

  // Pass pipeline the vertex position and normal in eye coordinates for light computation
  a_vertex_mv = modelview_matrix * vec4(b_vertex.x, b_vertex.y, b_vertex.z, 1.0);
  a_normal_mv = normalize(normal_matrix * a_normal);

  pos = b_vertex;

  // Apply full MVP transformation
  gl_Position = projection_matrix * a_vertex_mv;
}

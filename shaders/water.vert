#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

uniform float elapsed;

in vec3 a_vertex;
in vec2 a_texture;
in vec3 a_normal;

out vec4 a_vertex_mv;
out vec3 a_normal_mv;
out vec2 a_tex_coord;

#define waveWidth 0.6

void main()
{

	float newElapsed = elapsed/1000.0;
	float newy = (sin(waveWidth * a_vertex.x + newElapsed) * cos(waveWidth * a_vertex.y + newElapsed) * 0.3);

  // Pass pipeline the vertex position and normal in eye coordinates for light computation
  a_vertex_mv = modelview_matrix * vec4(a_vertex.x, newy, a_vertex.z, 1.0);
  a_normal_mv = normalize(normal_matrix * a_normal);

  


  // Texture coordinates 
  a_tex_coord = a_texture;

  // Apply full MVP transformation
  gl_Position = projection_matrix * a_vertex_mv;
}

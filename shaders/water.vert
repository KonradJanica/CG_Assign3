#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 a_vertex;
out vec4 a_vertex_mv;
out vec4 colour;


// Variables from http://jayconrod.com/posts/34/water-simulation-in-glsl


const float pi = 3.14159;
uniform float waterHeight;
uniform float time;
uniform float numWaves;
uniform float amplitude[8];
uniform float wavelength[8];
uniform float speed[8];
uniform vec2 direction[8];

float rand(vec2 n)
{
    return 0.5 + 0.5 * fract(sin(n.x*12.9898 + n.y*78.233)* 43758.5453);
}

float wave(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    return (amplitude[i]/2.0) * sin(theta * frequency + (time/10000.0) * phase);
}

float waveHeight(float x, float y) {
    float height = 0.0;
    for (int i = 0; i < numWaves; ++i)
        height += wave(i, x, y);
    return height;
}



void main()
{
	
	float h = a_vertex.y + 0.5 * ( 0.5 * waveHeight(a_vertex.x, a_vertex.z) );// + (0.5 * waveHeight(a_vertex.x, a_vertex.z));

	if(waveHeight(a_vertex.x, a_vertex.y) == 0.0)
	{
		colour = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		colour = vec4(0.0, 0.0, 1.0, 1.0);
	}
	a_vertex_mv = modelview_matrix * vec4(a_vertex.x, h, a_vertex.z, 1.0);
	// Apply full MVP transformation
	gl_Position = projection_matrix * a_vertex_mv;
}
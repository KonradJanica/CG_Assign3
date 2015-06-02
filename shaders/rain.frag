#version 130

in vec4 colour_f;

out vec4 fragColour;

void main()
{
	vec4 newcol = colour_f;
	newcol.a = 0.3;
  	fragColour = newcol;
}
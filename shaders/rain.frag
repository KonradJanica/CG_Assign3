#version 130

in vec4 colour_f;

out vec4 fragColour;

void main()
{
	// Set the colour as the apssed in vertex colour
	vec4 out_colour = colour_f;
	// Set the transparency to be low so the rain does not completely obscure view
	out_colour.a = 0.2;
  	fragColour = out_colour;
}
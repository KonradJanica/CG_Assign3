/*********************************************************
Version 1.000

Code provided by Michael Hemsley and Anthony Dick
for the course 
COMP SCI 3014/7090 Computer Graphics
School of Computer Science
University of Adelaide

Permission is granted for anyone to copy, use, modify, or distribute this
program and accompanying programs and documents for any purpose, provided
this copyright notice is retained and prominently displayed, along with
a note saying that the original programs are available from the aforementioned 
course web page. 

The programs and documents are distributed without any warranty, express or
implied.  As the programs were written for research purposes only, they have
not been tested to the degree that would be advisable in any important
application.  All use of these programs is entirely at the user's own risk.
*********************************************************/

#version 150

// manyAttributes.fp
// An example of using interpolated values from the previous stage

				// Take note that _per-vertex attributes_ are sent from previous stage
				// whereas uniforms are available for access in both stages

in vec2 text_coord;
uniform sampler2D texMap;

in vec4 litColour;

out vec4 fragColour;

void main(void) {


	//fragColour = texture(texMap, text_coord);	// We now just have to set the already smoothed colour as our frag colour
	fragColour = litColour*texture(texMap, text_coord);	// We now just have to set the already smoothed colour as our frag colour


}

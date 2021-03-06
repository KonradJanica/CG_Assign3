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

#version 130

// manyAttributes.vp
// This shader passes on colour values to be interpolated by the fixed functionality
// 

uniform mat4 mvp_matrix;

in vec3 a_vertex;

// This colour attribute will be sent out per-vertex and the fixed-functionality
// interpolates the colour (smooths the colour between) with connected vertices in the triangle
out vec4 colour;

void main(void) {
	
  // Doing the following in fragment shader instead
   colour.x = 0;
   colour.y = 0;
   colour.z = 0;

  if (a_vertex.x != 0) {
    colour.x = 1;
  } else if (a_vertex.y != 0) {  //Make Yellow colour for Y axis
    colour.y = 1;
  } else if (a_vertex.z != 0) {
    colour.z = 1;
  }

	gl_Position = mvp_matrix * vec4(a_vertex, 1.0);

}

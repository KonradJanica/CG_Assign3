#version 150

// manyAttributes.fp
// An example of using interpolated values from the previous stage

in vec4 colour; 
in vec3 texCoords;

uniform samplerCube skybox;

out vec4 fragColour;

void main(void) {

	// Force to make yellow
	//fragColour = vec4(1.0, 1.0, 0.0, 1.0);	// We now just have to set the already smoothed colour as our frag colour
	fragColour = texture(skybox, texCoords);
}

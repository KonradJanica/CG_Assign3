#version 150

// manyAttributes.fp
// An example of using interpolated values from the previous stage

in vec4 colour; 
in vec3 texCoords;

uniform samplerCube skybox;

out vec4 fragColour;

void main(void) {

	fragColour = texture(skybox, texCoords);
}

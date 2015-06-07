#version 130

in vec4 colour; 
in vec3 texCoords;

uniform samplerCube texMap;

out vec4 fragColour;

void main(void) {

	fragColour = texture(texMap, texCoords);
}

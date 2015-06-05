#version 130

// Ouput data
out float fragmentdepth;

void main(){
	fragmentdepth = gl_FragCoord.z;
}

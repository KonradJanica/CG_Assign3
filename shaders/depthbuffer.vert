#version 130

// Input vertex data, different for all executions of this shader.
//layout(location = 0) in vec3 vertexPosition_modelspace;
in vec3 a_vertex;

// Values that stay constant for the whole mesh.
uniform mat4 depth_mvp;

void main(){
	gl_Position = depth_mvp * vec4(a_vertex,1);
}

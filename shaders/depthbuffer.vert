#version 130

// Input vertex data, different for all executions of this shader.
in vec3 a_vertex;

// Values that stay constant for the whole mesh.
uniform mat4 depth_mvp_matrix;

void main(){
	gl_Position = depth_mvp_matrix * vec4(a_vertex,1);
}

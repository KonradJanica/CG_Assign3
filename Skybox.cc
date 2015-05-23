/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * Skybox.cc, Simple iplementation of a skybox in openGL
 * 
 * This file is a simple implementation of a skybox in openGL
 * for more clarification read the comments describing each function
 *
 * Inspiration drawn from http://learnopengl.com/#!Advanced-OpenGL/Cubemaps
 * 
 */

#include "Skybox.h"

Skybox::Skybox(const GLuint &program_id)
{
	// Set the class shader variable
	skybox_shader_ = program_id;

	// Create the VAO
	skybox_vao_ = CreateVao();

	// Create the Skybox texture cubemap
	std::vector<const GLchar*> faces;
	/* 	
		GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front
	*/
  faces.push_back("textures/morose_right.jpg"); // Left side of Cube
  faces.push_back("textures/morose_left.jpg");	// Right side of Cube
  faces.push_back("textures/morose_top.jpg");	// Top of cube
  faces.push_back("textures/rock02.jpg"); 		// Bottom of cube
  faces.push_back("textures/morose_front.jpg"); // Front (side furthest from camera)
  faces.push_back("textures/morose_back.jpg"); 	// Back (side closest to camera)

  // Load in the textures
  skybox_tex_ = loadCubeTex(faces);  
}

// Create a Skybox VAO (Essentially just a simple -1 to 1 cube)
// @return - return the vao that has been created
unsigned int Skybox::CreateVao()
{
	glUseProgram(skybox_shader_);

	// 
	float cubeVertices[] = {
        
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
	};



	unsigned int vaoHandle;
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	int vertLoc = glGetAttribLocation(skybox_shader_, "a_vertex");

	if(vertLoc == -1)
	{
		printf("Couldnt get vertex location for skybox\n");
	}

	// Buffers to store position, colour and index data
	unsigned int buffer[1];
	glGenBuffers(1, buffer);

	// Set vertex position
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertLoc);
	glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
    // Un-bind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	return vaoHandle;
  
}

// Generate a cubemap texture
// @input - a vector containing the string names of the faces you want to be on the skybox
// @return - The texture handle of the cubemap
GLuint Skybox::loadCubeTex(std::vector<const GLchar*> faces)
{

	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int x,y,n;
	unsigned char* data;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for(GLuint i = 0; i < faces.size(); i++)
	{   
  		
  		data = stbi_load(
      faces[i], /*char* filepath */
      // "crate.jpg",
      &x, /*The address to store the width of the image*/
      &y, /*The address to store the height of the image*/
      &n  /*Number of channels in the image*/,
      0   /*Force number of channels if > 0*/
      );

      glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;

}

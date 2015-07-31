#pragma once
#ifndef MY_TEXTURE_H
#define MY_TEXTURE_H

// Include glew to get required OpenGL headers
#include <GL/glew.h>
// SOIL library to load image for texture
#include <SOIL/SOIL.h>

class Texture
{
public:
	// Program ID
	GLuint TextureID;

	// Constructor reads and builds the shader
	Texture(const char* textureImagePath)
	{
		// Create and bind the OpenGL texture object
		glGenTextures(1, &(this->TextureID));
		glBindTexture(GL_TEXTURE_2D, this->TextureID);

		// Texture wrapping setup
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load image with SOIL
		int width, height;
		unsigned char* image = SOIL_load_image(textureImagePath, &width, &height, 0, SOIL_LOAD_RGB);

		// Set the objects image data, and generate mipmaps
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Free the image memory and unbind the texture object
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);
	};

};

#endif

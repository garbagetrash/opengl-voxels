#pragma once
// Standard libraries
#include <vector>
// Include glew to get required OpenGL headers
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/normalize_dot.hpp>


class GrassTextureGen
{
public:

	GLuint TextureSize;
	GLuint TextureID;
	std::vector<unsigned char> image;

	GrassTextureGen(GLuint texSize)
	{
		this->TextureSize = texSize;
		this->generate();
		this->textureGen();
	}

private:

	GLvoid generate()
	{
		for (GLint x = 0; x < this->TextureSize; x++)
		{
			for (GLint y = 0; y < this->TextureSize; y++)
			{
				GLint green = (GLint)(128.0f + (40.0 * rand()) / RAND_MAX);
				if (green > 255) green = 255;
				if (green < 0) green = 0;

				// Only using green channel
				image.push_back(0);
				image.push_back(green);
				image.push_back(0);
			}
		}
	}

	GLvoid textureGen()
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

		// Set the objects image data, and generate mipmaps
		unsigned char *image = this->image.data();
		std::cout << image[140] << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->TextureSize, this->TextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Free the image memory and unbind the texture object
		this->image.clear();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

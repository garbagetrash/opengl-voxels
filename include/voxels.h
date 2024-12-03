#pragma once
// Standard libs
#include <vector>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// My includes
#include "shader.h"

enum material {
	STONE,
	DIRT
};

class Voxels
{
public:
	// Defined from (0, 0, 0), (1, 0, 0), ... (1, 1, 1)
	GLfloat vertices[216] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		// Left face
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		// Right face
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	GLuint VBO, VAO, instanceVBO;
	Shader shader;
	std::vector<glm::vec3> translations;

	// Constructor
	Voxels(Shader shader)
	{
		// Set our shader
		this->shader = shader;

		// Initialize geometry
		glGenVertexArrays(1, &(this->VAO));
		glGenBuffers(1, &(this->VBO));

		glBindVertexArray(this->VAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		
		glBindVertexArray(0);
	}

	GLvoid addVoxel(glm::vec3 offset)
	{
		this->translations.push_back(offset);
		recomputeInstances();
	}

	GLvoid addVoxels(std::vector<glm::vec3> offsets)
	{
		std::vector<glm::vec3>::iterator it;
		it = this->translations.begin();
		this->translations.insert(it, offsets.begin(), offsets.end());
		recomputeInstances();
	}

	GLvoid popVoxel()
	{
		this->translations.pop_back();
		recomputeInstances();
	}

	GLvoid removeVoxel(GLuint index)
	{
		this->translations.erase(this->translations.begin() + index);
		recomputeInstances();
	}

	GLvoid recomputeInstances()
	{
		this->shader.Use();

		GLint location = glGetUniformLocation(this->shader.Program, "instanceSize");
		glUniform1i(location, this->translations.size());

		for (GLuint i = 0; i < this->translations.size(); i++)
		{
			std::stringstream ss;
			std::string index;
			ss << i;
			index = ss.str();
			location = glGetUniformLocation(this->shader.Program, ("offsets[" + index + "]").c_str());
			glUniform3f(location, this->translations[i].x, this->translations[i].y, this->translations[i].z);
		}

		glBindVertexArray(this->VAO);
		glGenBuffers(1, &(this->instanceVBO));
		glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * this->translations.size(), &(this->translations[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribDivisor(2, 1);
		glBindVertexArray(0);
	}

	GLvoid render()
	{
		this->shader.Use();
		glBindVertexArray(this->VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, this->translations.size());
		glBindVertexArray(0);
	}

	GLvoid deleteVoxels()
	{
		this->translations.clear();
		glDeleteVertexArrays(1, &(this->VAO));
		glDeleteBuffers(1, &(this->VBO));
	}

private:

};

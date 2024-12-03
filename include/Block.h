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
// My libraries
#include "marchingCubes.h"

typedef struct {
	glm::vec3 pos;
	GLfloat val;
} Voxel;

class Block
{
public:
	glm::vec3 pos;
	GLuint N;
	GLfloat gridsize;
	GLfloat(*densityFunc)(GLfloat, GLfloat, GLfloat);
	std::vector<Voxel> voxels;

	Block(glm::vec3 pos, GLuint N, GLfloat gridsize, GLfloat(*densityFunc)(GLfloat, GLfloat, GLfloat))
	{
		// Setup data
		this->pos = pos;
		this->N = N;
		this->gridsize = gridsize;
		this->densityFunc = densityFunc;

		// Initialize block
		initBlock();
	}

	GLvoid polygonize(std::vector<GLfloat>& vertices)
	{
		for (GLuint i = 0; i < this->voxels.size(); i++)
		{
			Cube(this->voxels[i].pos.x, this->voxels[i].pos.y, this->voxels[i].pos.z, this->gridsize, this->densityFunc, vertices);
		}
	}

private:

	GLvoid initBlock()
	{
		for (GLuint i = 0; i < N; i++)
		{
			for (GLuint j = 0; j < N; j++)
			{
				for (GLuint k = 0; k < N; k++)
				{
					Voxel temp;
					temp.pos = glm::vec3(this->pos.x + this->gridsize * ((GLfloat)i),
						this->pos.y + this->gridsize * ((GLfloat)j),
						this->pos.z + this->gridsize * ((GLfloat)k));
					temp.val = this->densityFunc(temp.pos.x, temp.pos.y, temp.pos.z);
					this->voxels.push_back(temp);
				}
			}
		}
	}

};

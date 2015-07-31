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

GLint HIGH_LOD_DIST = 

struct Voxel {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat value;
};

class VoxelField
{
public:
	glm::vec3 center;
	Voxel voxels[512][512][512];

	VoxelField(GLfloat x, GLfloat y, GLfloat z, GLfloat(*densityFunc)(GLfloat, GLfloat, GLfloat))
	{
		x = floor(x);
		y = floor(y);
		z = floor(z);
		GLint offset = HIGH_LOD_DIST / 2;
		for (GLint i = -HIGH_LOD_DIST; i < HIGH_LOD_DIST; i++)
		{
			for (GLint j = -HIGH_LOD_DIST; j < HIGH_LOD_DIST; j++)
			{
				for (GLint k = -HIGH_LOD_DIST; k < HIGH_LOD_DIST; k++)
				{
					Voxel newVox;
					newVox.x = i + x;
					newVox.y = j + z;
					newVox.z = k + z;
					newVox.value = densityFunc(newVox.x, newVox.y, newVox.z);
					this->voxels[i + offset][j + offset][k + offset] = newVox;
				}
			}
		}
	}

private:

};

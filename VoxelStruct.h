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
// My classes
#include "marchingCubes.h"

template <typename T>
class array3d {
public:
	array3d(GLsizei d1 = 0, GLsizei d2 = 0, GLsizei d3 = 0, T const & t = T()) :
		d1(d1), d2(d2), d3(d3), data(d1*d2*d3, t)
	{}

	T & operator()(GLsizei i, GLsizei j, GLsizei k)
	{
		return data[i*d2*d3 + j*d3 + k];
	}

	T const & operator()(GLsizei i, GLsizei j, GLsizei k) const
	{
		return data[i*d2*d3 + j*d3 + k];
	}

private:
	GLsizei d1, d2, d3;
	std::vector<T> data;
};

class VoxelStruct
{
public:
	GLuint N;
	array3d<GLfloat> voxelArr;
	glm::vec3 cameraPos;
	GLfloat(*densityFunc)(GLfloat, GLfloat, GLfloat);
	GLfloat gridsize;

	VoxelStruct(glm::vec3 pos, GLuint N, GLfloat(*densityFunc)(GLfloat, GLfloat, GLfloat), std::vector<GLfloat>& vertices, GLfloat gridsize)
	{
		this->cameraPos = pos;
		this->N = N;
		this->voxelArr = array3d<GLfloat> (2 * N + 1, 2 * N + 1, 2 * N + 1);
		this->densityFunc = *densityFunc;
		this->cameraIdxPtr = glm::vec3(0, 0, 0);
		this->gridsize = gridsize;

		init(pos, vertices);
	}

	GLvoid setPosition(glm::vec3 pos)
	{
		this->cameraPos = pos;
	}

	GLvoid updateVoxels(glm::vec3 newPos, std::vector<GLfloat>& vertices)
	{
		GLint x1, y1, z1, x2, y2, z2;
		x1 = round(this->cameraPos.x);
		y1 = round(this->cameraPos.y);
		z1 = round(this->cameraPos.z);
		x2 = round(newPos.x);
		y2 = round(newPos.y);
		z2 = round(newPos.z);
		GLint dx, dy, dz;
		dx = x2 - x1;
		dy = y2 - y1;
		dz = z2 - z1;

		GLint N = this->N;
		glm::vec3 idxPtr = this->cameraIdxPtr;
		// X ray movement...
		GLint signX = (dx > 0) ? 1 : -1;
		for (GLint x = 0; x < abs(dx); x++)
		{
			for (GLint y = 0; y < 2 * N + 1; y++)
			{
				for (GLint z = 0; z < 2 * N + 1; z++)
				{
					GLint xpos = signX * (x + 1 + N) + x1;
					GLint ypos = y - N + y1;
					GLint zpos = z - N + z1;
					GLint xIdx = idxPtr.x + signX * (x + 1);
					while (xIdx < 0)
					{
						xIdx += 2 * N + 1;
					}
					while (xIdx >= 2 * N + 1)
					{
						xIdx -= 2 * N + 1;
					}
					this->voxelArr(xIdx, y, z) = this->densityFunc(xpos, ypos, zpos);

					// Now polygonize!
					this->polygonize(xpos, ypos, zpos, vertices);
				}
			}
		}

		// Y ray movement...
		GLint signY = (dy > 0) ? 1 : -1;
		for (GLint x = 0; x < 2 * N + 1; x++)
		{
			for (GLint y = 0; y < abs(dy); y++)
			{
				for (GLint z = 0; z < 2 * N + 1; z++)
				{
					GLint xpos = x - N + x1;
					GLint ypos = signY * (y + 1 + N) + y1;
					GLint zpos = z - N + z1;
					GLint yIdx = idxPtr.y + signY * (y + 1);
					while (yIdx < 0)
					{
						yIdx += 2 * N + 1;
					}
					while (yIdx >= 2 * N + 1)
					{
						yIdx -= 2 * N + 1;
					}
					this->voxelArr(x, yIdx, z) = this->densityFunc(xpos, ypos, zpos);

					// Now polygonize!
					this->polygonize(xpos, ypos, zpos, vertices);
				}
			}
		}

		// Z ray movement...
		GLint signZ = (dz > 0) ? 1 : -1;
		for (GLint x = 0; x < 2 * N + 1; x++)
		{
			for (GLint y = 0; y < 2 * N + 1; y++)
			{
				for (GLint z = 0; z < abs(dz); z++)
				{
					GLint xpos = x - N + x1;
					GLint ypos = y - N + y1;
					GLint zpos = signZ * (z + 1 + N) + z1;
					GLint zIdx = idxPtr.z + signZ * (z + 1);
					while (zIdx < 0)
					{
						zIdx += 2 * N + 1;
					}
					while (zIdx >= 2 * N + 1)
					{
						zIdx -= 2 * N + 1;
					}
					this->voxelArr(x, y, zIdx) = this->densityFunc(xpos, ypos, zpos);

					// Now polygonize!
					this->polygonize(xpos, ypos, zpos, vertices);
				}
			}
		}

		// When new voxels are figured out, update camera absolute position and index pointer.
		GLint xidx = idxPtr.x + dx;
		GLint yidx = idxPtr.y + dy;
		GLint zidx = idxPtr.z + dz;
		while (xidx < 0)
		{
			xidx += 2 * N + 1;
		}
		while (xidx >= 2 * N + 1)
		{
			xidx -= 2 * N + 1;
		}
		while (yidx < 0)
		{
			yidx += 2 * N + 1;
		}
		while (yidx >= 2 * N + 1)
		{
			yidx -= 2 * N + 1;
		}
		while (zidx < 0)
		{
			zidx += 2 * N + 1;
		}
		while (zidx >= 2 * N + 1)
		{
			zidx -= 2 * N + 1;
		}
		this->cameraIdxPtr = glm::vec3(xidx, yidx, zidx);
		this->cameraPos = newPos;
	}

private:
	glm::vec3 cameraIdxPtr;

	GLvoid init(glm::vec3 pos, std::vector<GLfloat>& vertices)
	{
		GLint N = this->N;
		pos = glm::vec3(round(pos.x), round(pos.y), round(pos.z));
		for (GLint x = 0; x < 2 * N + 1; x++)
		{
			for (GLint y = 0; y < 2 * N + 1; y++)
			{
				for (GLint z = 0; z < 2 * N + 1; z++)
				{
					this->voxelArr(x, y, z) = this->densityFunc(x - N + pos.x, y - N + pos.y, z - N + pos.z);
					
					// Now polygonize!
					this->polygonize(x - N + pos.x, y - N + pos.y, z - N + pos.z, vertices);
				}
			}
		}
	}

	GLvoid polygonize(GLfloat x, GLfloat y, GLfloat z, std::vector<GLfloat>& vertices)
	{
		Cube(x, y, z, 1.0f, this->densityFunc, vertices);
	}

	GLvoid LODpolygonize(GLfloat x, GLfloat y, GLfloat z, std::vector<GLfloat>& vertices)
	{
		Cube(x, y, z, 2.0f, this->densityFunc, vertices);
	}

};

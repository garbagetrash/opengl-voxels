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

typedef struct {
	GLint x;
	GLint y;
	GLint z;
} IntPoint;

class VoxelOctree
{
public:
	IntPoint p;
	GLuint level;		// Level 0 is lowest, it has 8 bytes and represents a 4x4x4 chunk of voxels. Level 1 is 8x8x8, level 2 is 16x16x16, ect...
	std::vector<VoxelOctree> children;
	std::vector<GLbyte> blocks;

	// Let our octree be centered on the origin in world space (for now.)

	// Use byte to represent 8 blocks in cube. Coordinates (x,y,z) in cube of block represents which bit.
	// SO, the fore, bottom left block is (0, 0, 0) and so is represented by the 0th bit (LSB) in the byte.
	// The rear, bottom left block is (0, 0, 1) and is the 1st bit.  The 4th bit is (1, 0, 0) ie the fore, bottom, right block.
	// The byte 0x13 has bits 0, 1, and 4 set, so it implies that the cube has the fore, bottom left and right blocks set,
	// and the rear bottom left block as well.

	VoxelOctree(GLint x, GLint y, GLint z, GLuint level)
	{
		this->p.x = x;
		this->p.y = y;
		this->p.z = z;
		this->level = level;

		if (level > 0)
		{
			this->initChildren();
		}
		else
		{
			// Lowest level
			this->initBlocks();
		}
	}

	// Set the block corresponding to (x, y, z) in world space
	GLvoid set(GLint x, GLint y, GLint z)
	{
		GLint offset = pow(2, this->level + 1);
		GLint xidx = (x - this->p.x >= offset) ? 1 : 0;
		GLint yidx = (y - this->p.y >= offset) ? 1 : 0;
		GLint zidx = (z - this->p.z >= offset) ? 1 : 0;
		if (this->level > 0)
		{
			this->children[4 * xidx + 2 * yidx + zidx].set(x, y, z);
		}
		else
		{
			GLbyte value = 0;
			if ((x - this->p.x) % 2 == 1) value += 4;
			if ((y - this->p.y) % 2 == 1) value += 2;
			if ((z - this->p.z) % 2 == 1) value += 1;
			this->blocks[4 * xidx + 2 * yidx + zidx] |= (1 << value);
		}
	}

	// Assumes x, y, z will always be either 0, or 1.
	GLbyte get(GLint x, GLint y, GLint z)
	{
		GLint offset = pow(2, this->level + 1);
		GLint xidx = (x - this->p.x >= offset) ? 1 : 0;
		GLint yidx = (y - this->p.y >= offset) ? 1 : 0;
		GLint zidx = (z - this->p.z >= offset) ? 1 : 0;
		if (this->level > 0)
		{
			return this->children[4 * xidx + 2 * yidx + zidx].get(x, y, z);
		}
		else
		{
			return this->blocks[4 * xidx + 2 * yidx + zidx];
		}
	}

private:

	GLvoid initChildren()
	{
		GLint offset = pow(2, this->level + 1);
		this->children.push_back(VoxelOctree(this->p.x,          this->p.y,          this->p.z,          this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x,          this->p.y,          this->p.z + offset, this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x,          this->p.y + offset, this->p.z,          this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x,          this->p.y + offset, this->p.z + offset, this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x + offset, this->p.y,          this->p.z,          this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x + offset, this->p.y,          this->p.z + offset, this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x + offset, this->p.y + offset, this->p.z,          this->level - 1));
		this->children.push_back(VoxelOctree(this->p.x + offset, this->p.y + offset, this->p.z + offset, this->level - 1));
	}

	GLvoid initBlocks()
	{
		for (GLuint i = 0; i < 8; i++)
		{
			this->blocks.push_back(0x00);
		}
	}

};

#pragma once
// Standard libraries
#include <vector>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/normalize_dot.hpp>

class Noise
{
public:


	Noise()
	{
		for (GLuint i = 0; i < 512; i++)
		{
			pMod12[i] = (GLubyte) (p[i] % 12);
		}
	}

	GLfloat perlinImproved(GLfloat x, GLfloat y, GLfloat z)
	{
		// Find the unit cube that contains the point (x, y, z)
		GLint X = (GLint)floor(x) & 255;
		GLint Y = (GLint)floor(y) & 255;
		GLint Z = (GLint)floor(z) & 255;

		// Find the relative (x, y, z) of point in cube
		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		// Compute fade curves for each of (x, y, z)
		GLfloat u = this->fade(x);
		GLfloat v = this->fade(y);
		GLfloat w = this->fade(z);

		// Hash coordinates of the 8 cube corners
		GLint A  = this->p[X] + Y;
		GLint AA = this->p[A] + Z;
		GLint AB = this->p[A+1] + Z;
		GLint B  = this->p[X+1] + Y;
		GLint BA = this->p[B] + Z;
		GLint BB = this->p[B+1] + Z;

		// And add blended results from 8 corners of cube
		return this->lerp(w, this->lerp(v, this->lerp(u, this->grad(this->p[AA  ], x  , y  , z),
														 this->grad(this->p[BA  ], x-1, y  , z)),
										   this->lerp(u, this->grad(this->p[AB  ], x  , y-1, z),
														 this->grad(this->p[BB  ], x-1, y-1, z))),
							 this->lerp(v, this->lerp(u, this->grad(this->p[AA+1], x  , y  , z-1),
														 this->grad(this->p[BA+1], x-1, y  , z-1)),
										   this->lerp(u, this->grad(this->p[AB+1], x  , y-1, z-1),
														 this->grad(this->p[BB+1], x-1, y-1, z-1))));
	}

	GLfloat simplex2d(GLfloat xin, GLfloat yin)
	{
		// Noise contributions from the 3 corners
		GLfloat n0, n1, n2;
		// Hairy factor for 2D
		GLfloat s = (xin + yin) * F2;
		GLint i = fastfloor(xin + s);
		GLint j = fastfloor(yin + s);
		GLfloat t = (i + j) * G2;
		// Unskew the cell origin back to (x, y) space
		GLfloat X0 = i - t;
		GLfloat Y0 = j - t;
		GLfloat x0 = xin - X0;
		GLfloat y0 = yin - Y0;
		// For the 2D case, the simplex shape is an equilateral triangle.
		// Determine which simplex we are in.
		GLint i1, j1; // Offsets for second (middle) corner of simplex in (i, j) coords
		if (x0 > y0)
		{
			// Lower triangle, XY order: (0, 0) -> (1, 0) -> (1, 1)
			i1 = 1;
			j1 = 0;
		}
		else
		{
			// Upper triangle, YX order: (0, 0) -> (0, 1) -> (1, 1)
			i1 = 0;
			j1 = 1;
		}
		// A step of (1, 0) in (i, j) means a step of (1 - c, -c) in (x, y), and
		// a step of (0, 1) in (i, j) means a step of (-c, 1 - c) in (x, y) where
		// c = (3 - sqrt(3)) / 6
		GLfloat x1 = x0 - i1 + G2;	// Offsets for middle corner in (x, y) unskewed coords
		GLfloat y1 = y0 - j1 + G2;
		GLfloat x2 = x0 - 1.0 + 2.0 * G2;	// Offsets for last corner in (x, y) unskewed coords
		GLfloat y2 = y0 - 1.0 + 2.0 * G2;

		// Work out the hashed gradient indices of the 3 simplex corners
		GLint ii = i & 255;
		GLint jj = j & 255;
		GLint gi0 = pMod12[ii + p[jj]];
		GLint gi1 = pMod12[ii + i1 + p[jj + j1]];
		GLint gi2 = pMod12[ii + 1 + p[jj + 1]];
		// Calculate the contribution from the 3 corners
		GLfloat t0 = 0.5 - x0 * x0 - y0 * y0;
		if (t0 < 0)
		{
			n0 = 0.0;
		}
		else
		{
			t0 *= t0;
			n0 = t0 * t0 * dot(grad3[gi0], x0, y0);	// (x, y) of grad3 used for 2D gradient
		}
	}

private:
	GLubyte p[512] = {
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	GLubyte pMod12[512];

	// Skewing and unskewing factors for 2, 3, and 4 dimensions
	GLfloat F2 = 0.5 * (sqrt(3.0) - 1.0);
	GLfloat G2 = (3.0 - sqrt(3.0)) / 6.0;
	GLfloat F3 = 1.0 / 3.0;
	GLfloat G3 = 1.0 / 6.0;
	GLfloat F4 = (sqrt(5.0) - 1.0) / 4.0;
	GLfloat G4 = (5.0 - sqrt(5.0)) / 20.0;

	// Inner class to speed up gradient computations
	class Grad
	{
	public:
		GLfloat x, y, z, w;

		Grad(GLfloat x, GLfloat y, GLfloat z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Grad(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

	private:

	};

	Grad grad3[12] = { Grad(1, 1, 0), Grad(-1, 1, 0), Grad(1, -1, 0), Grad(-1, -1, 0),
					   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
					   Grad(0, 1, 1), Grad(0, -1, 1), Grad(0, 1, -1), Grad(0, -1, -1) };

	// TODO: REAL VALUES HERE
	/*
	Grad grad4[32] = { Grad(1, 1, 0), Grad(-1, 1, 0), Grad(1, -1, 0), Grad(-1, -1, 0),
					   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
					   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
					   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
		  			   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
					   Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
					   Grad(0, 1, 1), Grad(0, -1, 1), Grad(0, 1, -1), Grad(0, -1, -1) };*/

	// Fast method of flooring to an int.
	GLint fastfloor(GLfloat x)
	{
		GLint xi = (GLint) x;
		return x < xi ? xi - 1 : xi;
	}

	GLfloat dot(Grad g, GLfloat x, GLfloat y)
	{
		return g.x*x + g.y*y;
	}

	GLfloat dot(Grad g, GLfloat x, GLfloat y, GLfloat z)
	{
		return g.x*x + g.y*y + g.z*z;
	}

	GLfloat dot(Grad g, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
	{
		return g.x*x + g.y*y + g.z*z + g.w*w;
	}

	GLfloat grad(GLint hash, GLfloat x, GLfloat y, GLfloat z)
	{
		GLint h = hash & 15;	// Convert low 4 bits of hash code
		float u = (h < 8) ? x : y;
		float v = (h < 4) ? y : (h == 12 || h == 14) ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	GLfloat lerp(GLfloat t, GLfloat a, GLfloat b)
	{
		return a + (b - a) * t;
	}

	GLfloat fade(GLfloat t)
	{
		return t*t*t*(6.0f*t*t - 15.0f*t + 10.0f);
	}

};

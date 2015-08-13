// SimplexNoise1234
// Copyright © 2003-2011, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
//
// This library is public domain software, released by the author
// into the public domain in February 2011. You may do anything
// you like with it. You may even remove all attributions,
// but of course I'd appreciate it if you kept my name somewhere.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

/** \file
\brief Declares the SimplexNoise1234 class for producing Perlin simplex noise.
\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
* This is a clean, fast, modern and free Perlin Simplex noise class in C++.
* Being a stand-alone class with no external dependencies, it is
* highly reusable without source code modifications.
*
*
* Note:
* Replacing the "float" type with "double" can actually make this run faster
* on some platforms. A templatized version of SimplexNoise1234 could be useful.
*/

class SimplexNoise1234 {

public:
	SimplexNoise1234() {}
	~SimplexNoise1234() {}

	/** 1D, 2D, 3D and 4D float Perlin noise
	*/
	static GLfloat SimplexNoise1234::noise(GLfloat x);
	static GLfloat SimplexNoise1234::noise(GLfloat x, GLfloat y);
	static GLfloat SimplexNoise1234::noise(GLfloat x, GLfloat y, GLfloat z);
	static GLfloat SimplexNoise1234::noise(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

	/** 1D, 2D, 3D and 4D float Perlin noise, with a specified integer period
	*/
	static GLfloat SimplexNoise1234::pnoise(GLfloat x, GLint px);
	static GLfloat SimplexNoise1234::pnoise(GLfloat x, GLfloat y, GLint px, GLint py);
	static GLfloat SimplexNoise1234::pnoise(GLfloat x, GLfloat y, GLfloat z, GLint px, GLint py, GLint pz);
	static GLfloat SimplexNoise1234::pnoise(GLfloat x, GLfloat y, GLfloat z, GLfloat w,
		GLint px, GLint py, GLint pz, GLint pw);

private:
	static unsigned char SimplexNoise1234::perm[];
	static GLfloat  SimplexNoise1234::grad(GLint hash, GLfloat x);
	static GLfloat  SimplexNoise1234::grad(GLint hash, GLfloat x, GLfloat y);
	static GLfloat  SimplexNoise1234::grad(GLint hash, GLfloat x, GLfloat y, GLfloat z);
	static GLfloat  SimplexNoise1234::grad(GLint hash, GLfloat x, GLfloat y, GLfloat z, GLfloat t);

};

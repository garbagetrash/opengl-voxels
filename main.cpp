/////////////////////
// Includes
// Standard libraries
#include <iostream>
#include <math.h>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// SOIL
#include <SOIL/SOIL.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// My files
#include "shader.h"
#include "mesh.h"
#include "noise.h"
#include "grassTex.h"
#include "camera.h"
#include "voxels.h"
//#include "marchingCubes.h"
#include "VoxelOctree.h"

//////////
// Globals
GLboolean wireframeMode = GL_FALSE;
const GLuint screenWidth = 1680, screenHeight = 1050;
// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
GLfloat lastX = screenWidth / 2.0f, lastY = screenHeight / 2.0f;
GLboolean keys[1024];
// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;	// Time of last frame
GLfloat accumTime = 0.0f;

//////////////////////
// Function prototypes
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);
GLvoid mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
GLvoid key_callback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mode);
GLvoid do_movement(GLvoid);
GLvoid collisionDetect(std::vector<GLfloat> vertices);
GLboolean pointInTriag(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
GLboolean sameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b);
GLFWwindow* initWindow(GLvoid);
GLfloat sphereDist(GLfloat x, GLfloat y, GLfloat z);

////////////////
// Main function
int main()
{
	/////////////////////////////////////////////
	// Initialize GLFW, GLEW, and create a window
	GLFWwindow* window = initWindow();
	if (window == nullptr)
	{
		return -1;
	}

	//////////////////////////////////////
	// Set the required callback functions
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	///////////////////////////////////////
	// Build and compile our shader program
	//Shader shader("voxels.vs", "voxels.frag");
	Shader simpleShader("simpleShader.vs", "simpleShader.frag");
	Shader grassShader("grassShader.vs", "grassShader.frag");

	GLint N = 32;
	GLfloat GRID_SIZE = 0.5f;
	// Evaluate the grid with our distance field.
	std::vector<GLfloat> vertices;
	//vertices.resize(8 * 3 * 4 * N * N);
	//std::vector<Vertex> vertices2;
	std::vector<GLuint> indices;
	GLfloat fake[] = {
		// Point			// Normal         // Tex Coords
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
	};

	for (GLuint i = 0; i < 24; i++)
	{
		vertices.push_back(fake[i]);
	}

	//vertices.clear();
	
	VoxelOctree cell(-16.0f, 0.0f, -16.0f, 5, sphereDist, vertices);
	std::cout << vertices.capacity() << std::endl;
	std::cout << vertices.max_size() << std::endl;
	std::cout << vertices.size() << std::endl;

	/*
	for (GLfloat x = 0; x < N; x++)
	{
		for (GLfloat y = 0; y < N; y++)
		{
			for (GLfloat z = 0; z < N; z++)
			{
				Cube cube(x*GRID_SIZE, y*GRID_SIZE, z*GRID_SIZE, GRID_SIZE);
				for (GLint i = 0; i < 8; i++)
				{
					cube.cornerVals[i] = sphereDist(cube.cornersCoords[i].x, cube.cornersCoords[i].y, cube.cornersCoords[i].z);

					GLfloat xdiff = (sphereDist(x*GRID_SIZE + GRID_SIZE, y*GRID_SIZE, z*GRID_SIZE) - sphereDist(x*GRID_SIZE - GRID_SIZE, y*GRID_SIZE, z*GRID_SIZE));
					GLfloat ydiff = (sphereDist(x*GRID_SIZE, y*GRID_SIZE + GRID_SIZE, z*GRID_SIZE) - sphereDist(x*GRID_SIZE, y*GRID_SIZE - GRID_SIZE, z*GRID_SIZE));
					GLfloat zdiff = (sphereDist(x*GRID_SIZE, y*GRID_SIZE, z*GRID_SIZE + GRID_SIZE) - sphereDist(x*GRID_SIZE, y*GRID_SIZE, z*GRID_SIZE - GRID_SIZE));
					cube.cornerNorms[i] = glm::normalize(glm::vec3(xdiff, ydiff, zdiff));
				}
				GLint nTriags = cube.polygonise(0.0f);
				for (GLint i = 0; i < nTriags; i++)
				{
					// Position
					vertices.push_back(cube.triangles[i].p[0].x);
					vertices.push_back(cube.triangles[i].p[0].y);
					vertices.push_back(cube.triangles[i].p[0].z);
					// Normal
					vertices.push_back(cube.triangles[i].n[0].x);
					vertices.push_back(cube.triangles[i].n[0].y);
					vertices.push_back(cube.triangles[i].n[0].z);
					// Tex Coords
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(cube.triangles[i].p[1].x);
					vertices.push_back(cube.triangles[i].p[1].y);
					vertices.push_back(cube.triangles[i].p[1].z);
					vertices.push_back(cube.triangles[i].n[1].x);
					vertices.push_back(cube.triangles[i].n[1].y);
					vertices.push_back(cube.triangles[i].n[1].z);
					vertices.push_back(1.0f);
					vertices.push_back(0.0f);
					vertices.push_back(cube.triangles[i].p[2].x);
					vertices.push_back(cube.triangles[i].p[2].y);
					vertices.push_back(cube.triangles[i].p[2].z);
					vertices.push_back(cube.triangles[i].n[2].x);
					vertices.push_back(cube.triangles[i].n[2].y);
					vertices.push_back(cube.triangles[i].n[2].z);
					vertices.push_back(1.0f);
					vertices.push_back(1.0f);

					/*
					for (GLuint j = 0; j < 3; j++)
					{
						Vertex v;
						v.Position = cube.triangles[i].p[j];
						v.Normal = cube.triangles[i].n[j];
						vertices2.push_back(v);
						indices.push_back(i * 3 + j);
					}
				}
			}
		}
	}
	*/
	std::vector<Texture> textures;
	//Mesh mesh(vertices2, indices, textures);

	std::cout << vertices.size() << std::endl;

	// Standard mesh stuff
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind vertex array object first, then bind and set vertex buffer(s) and attribute pointer(s)
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(fake), fake, GL_STATIC_DRAW);

	// Set the vertex attributes pointers
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Tex Coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// Unbind the VAO
	glBindVertexArray(0);

	// Texture generation
	grassShader.Use();
	GrassTextureGen grassGen(64);
	glUniform1i(glGetUniformLocation(grassShader.Program, "material.diffuse"), 0);

	////////////////////
	// Main program loop
	while (!glfwWindowShouldClose(window))
	{
		// Check for any events
		glfwPollEvents();
		do_movement();
		camera.collisionDetect(vertices, deltaTime);

		// Render code
		// Clear the color buffer
		glClearColor(0.7f, 0.7f, 0.85f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Load up the shader
		grassShader.Use();

		// Textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassGen.TextureID);

		// Time varying color, update the color uniform
		GLfloat timeValue = glfwGetTime();
		deltaTime = timeValue - lastFrame;
		lastFrame = timeValue;

		// Handle coordinate system - Vertex shader uniforms
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(grassShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(grassShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(grassShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Other uniforms
		glm::vec3 lightPos(256.0f, 1024.0f, 1024.0f);
		glUniform3f(glGetUniformLocation(grassShader.Program, "material.specular"), 0.1f, 0.1f, 0.1f);
		glUniform1f(glGetUniformLocation(grassShader.Program, "material.shininess"), 2.0f);

		glm::vec3 lightColor(1.0f, 0.9f, 0.9f);
		glm::vec3 diffuseC = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientC = diffuseC * glm::vec3(0.2f);
		glUniform3f(glGetUniformLocation(grassShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(grassShader.Program, "light.ambient"), ambientC.x, ambientC.y, ambientC.z);
		glUniform3f(glGetUniformLocation(grassShader.Program, "light.diffuse"), diffuseC.x, diffuseC.y, diffuseC.z);
		glUniform3f(glGetUniformLocation(grassShader.Program, "light.specular"), 1.0f, 1.0f, 1.0f);

		GLint viewPosLoc = glGetUniformLocation(grassShader.Program, "viewPos");

		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		// Draw container
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);
		glBindVertexArray(0);
		//mesh.Draw(simpleShader);

		// Swap the screen buffers
		glfwSwapBuffers(window);

		accumTime += deltaTime;
		if (accumTime > 3.0)
		{
			// Every 3 s draw new cell around player
			vertices.clear();
			VoxelOctree cell(camera.Position.x - 8.0f, 0.0f, camera.Position.z - 8.0f, 4, sphereDist, vertices);
			accumTime = 0.0f;
		}
	}
	// De-allocate all resources once they're done
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	
	// Terminate GLFW, clearing any resources allocated by GLFW
	glfwTerminate();
	return 0;
}

// Distance from a sphere of radius 10
GLfloat sphereDist(GLfloat x, GLfloat y, GLfloat z)
{
	//return 256.0f - pow(x - 16.0f, 2) - pow(y - 16.0f, 2) - pow(z - 16.0f, 2);
	static Noise n;
	return y - 4.2f
		     + 4.0f * n.perlinImproved(x / 16.0f, y / 16.0f, z / 16.0f)
		     + 2.0f * n.perlinImproved(x / 9.0f,  y / 9.0f,  z / 9.0f)
			 + 1.0f * n.perlinImproved(x / 4.0f,  y / 4.0f,  z / 4.0f)
		     + 0.5f * n.perlinImproved(x / 1.5f,  y / 1.5f,  z / 1.5f);
}

GLFWwindow* initWindow(GLvoid)
{
	GLint status = 0;

	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required optoins for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Voxel Stuff", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// Set this so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup the OpenGL function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return nullptr;
	}

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return window;
}

GLvoid do_movement()
{
	// Camera controls
	GLfloat dTime = deltaTime;
	if (keys[GLFW_KEY_LEFT_SHIFT]) { dTime *= 2.0f; }
	if (keys[GLFW_KEY_W]) { camera.ProcessKeyboard(FORWARD, dTime); }
	if (keys[GLFW_KEY_S]) { camera.ProcessKeyboard(BACKWARD, dTime); }
	if (keys[GLFW_KEY_A]) { camera.ProcessKeyboard(LEFT, dTime); }
	if (keys[GLFW_KEY_D]) { camera.ProcessKeyboard(RIGHT, dTime); }
}

GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

GLboolean firstMouse = GL_TRUE;
GLvoid mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = GL_FALSE;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

GLvoid key_callback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mode)
{
	// When a user presses the escape key, we set the WindowShouldClose property to true,
	// closing the application
	if (action == GLFW_PRESS)
	{
		keys[key] = GL_TRUE;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = GL_FALSE;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
	{
		if (wireframeMode == GL_FALSE)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframeMode = GL_TRUE;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			wireframeMode = GL_FALSE;
		}
	}
}

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
#include <glm/gtc/noise.hpp>
// OpenCL
#include <CL/cl_gl.h>
// My files
#include "shader.h"
#include "mesh.h"
#include "noise.h"
#include "simplexnoise1234.h"
#include "grassTex.h"
#include "camera.h"
#include "voxels.h"
#include "marchingCubes.h"
//#include "VoxelOctree.h"
//#include "VoxelStruct.h"
#include "Block.h"

/////////////////////
// OpenCL Kernel Code
const GLchar *KernelSource = "\n" \
"__kernel void vadd(\n" \
"  __global float* a,\n" \
"  __global float* b,\n" \
"  __global float* c,\n" \
"  const unsigned int cout)\n" \
"{\n" \
"  int i = get_global_id(0);\n" \
"  if (i < count)\n" \
"    c[i] = a[i] + b[i];\n" \
"}\n" \
"\n";
//-----------------------------------------------------

//////////
// Globals
GLboolean wireframeMode = GL_FALSE;
const GLuint screenWidth = 1680, screenHeight = 1050;
// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 10.0f));
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
GLint openClDeviceInfo(GLvoid);
GLint initCL(GLvoid);
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

	///////////////////////////////////////
	// Setup OpenCL
	openClDeviceInfo();

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
	
	// Initialize some terrain
	GLint N = 8;
	//VoxelStruct field(glm::vec3(0.0f, 0.0f, 0.0f), N, sphereDist, vertices, 1.0f);
	Block firstBlock(glm::vec3(0.0f, 0.0f, 0.0f), 64, 1.0f, sphereDist);
	firstBlock.polygonize(vertices);

	std::cout << vertices.capacity() << std::endl;
	std::cout << vertices.max_size() << std::endl;
	std::cout << vertices.size() << std::endl;

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
		GLfloat frameStart = glfwGetTime();

		// Check for any events
		glfwPollEvents();
		do_movement();
		//camera.collisionDetect(vertices, deltaTime);

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
		GLfloat frameEnd = glfwGetTime();
		//if (frameEnd - frameEnd < 0.01)
		//{
			// Every 3 s draw new cell around player
			//vertices.clear();
			GLuint nVertexes = 1000000;
			if (vertices.size() > 8 * nVertexes)
			{
				//vertices.clear();
			}
			//field.updateVoxels(camera.Position, vertices);
			//VoxelLodStruct(camera.Position, 1024.0f, 0, vertices);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
			//std::cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
			accumTime = 0.0f;
		//}
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
	//static Noise n;
	static SimplexNoise1234 n;
	return y*y - 4.0f
		//+ 16.0f * n.perlinImproved(x / 64.0f, y / 64.0f, z / 64.0f)
		+ 4.0f * n.noise(x / 16.0f, y / 16.0f, z / 16.0f)
		+ 2.0f * n.noise(x / 9.0f, y / 9.0f, z / 9.0f)
		+ 1.0f * n.noise(x / 4.0f, y / 4.0f, z / 4.0f)
		+ 0.5f * n.noise(x / 1.5f, y / 1.5f, z / 1.5f);
	/*
	return y - 4.2f
		+ 4.0f * glm::simplex(glm::vec3(x / 16.0f, y / 16.0f, z / 16.0f))
		+ 2.0f * glm::simplex(glm::vec3(x / 9.0f, y / 9.0f, z / 9.0f))
		+ 1.0f * glm::simplex(glm::vec3(x / 4.0f, y / 4.0f, z / 4.0f))
		+ 0.5f * glm::simplex(glm::vec3(x / 1.5f, y / 1.5f, z / 1.5f));
		*/
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

GLint initCL()
{
	GLfloat TOL = 0.001;
	GLint LENGTH = 1024;

	GLint err;

	std::vector<GLfloat> h_a(LENGTH, 0);
	std::vector<GLfloat> h_b(LENGTH, 0);
	std::vector<GLfloat> h_c(LENGTH, 0);

	cl_device_id device_id;
	cl_context context;
	cl_command_queue commands;
	cl_program program;
	cl_kernel ko_vadd;

	cl_mem d_a;
	cl_mem d_b;
	cl_mem d_c;

	// Fill vectors a and b with random floats
	GLint i = 0;
	GLint count = LENGTH;
	for (i = 0; i < count; i++)
	{
		h_a[i] = rand() / (GLfloat)RAND_MAX;
		h_b[i] = rand() / (GLfloat)RAND_MAX;
	}

	// Setup platform and GPU device
	cl_uint numPlatforms;

	// Find number of platforms
	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	//checkError(err, "Finding platforms");
	if (numPlatforms == 0)
	{
		std::cout << "Found 0 platforms!" << std::endl;
		return EXIT_FAILURE;
	}

	// Get all platforms
	std::vector<cl_platform_id> Platform(numPlatforms);
	err = clGetPlatformIDs(numPlatforms, Platform.data(), NULL);
	//checkError(err, "Getting platforms");

	// Secure a GPU
	for (i = 0; i < numPlatforms; i++)
	{
		err = clGetDeviceIDs(Platform[i], DEVICE, 1, &device_id, NULL);
		if (err == CL_SUCCESS)
		{
			break;
		}
	}

	if (device_id == NULL)
	{
		//checkError(err, "Finding a device");
	}

	err = output_device_info(device_id);
	//checkError(err, Printing a device output");

	// Create a compute context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	//checkError(err, "Creating a context");

	// Create a command queue
	commands = clCreateCommandQueue(context, device_id, 0, &err);
	//checkError(err, "Creating a command queue");

	// Create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const GLchar **)&KernelSource, NULL, &err);
	//checkError(err, "Creating a program");

	// Build the program
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len;
		GLchar buffer[2048];

		std::cout << "Error: Failed to biuld program executable!" << std::endl << err << std::endl;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		std::cout << buffer << std::endl;
		return EXIT_FAILURE;
	}

	// Create the compute kernel from the program
	ko_vadd = clCreateKernel(program, "vadd", &err);
	//checkError(err, "Creating kernel");

	// Create the input (a, b) and output (c) arrays in device memory
	d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * count, NULL, &err);
	//checkError(err, "Creating buffer d_a");
	d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * count, NULL, &err);
	//checkError(err, "Creating buffer d_b");
	d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, &err);
	//checkError(err, "Creating buffer d_c");

	// Write a and b vectors into compute device memory
	err = clEnqueueWriteBuffer(commands, d_a, CL_TRUE, 0, sizeof(float) * count, h_a.data(), 0, NULL, NULL);
	//checkError(err, "Copying h_a to device at d_a");
	err = clEnqueueWriteBuffer(commands, d_b, CL_TRUE, 0, sizeof(float) * count, h_b.data(), 0, NULL, NULL);
	//checkError(err, "Copying h_b to device at d_b");

	// Set the arguments to our compute kernel
	err = clSetKernelArg(ko_vadd, 0, sizeof(cl_mem), &d_a);
	err = clSetKernelArg(ko_vadd, 1, sizeof(cl_mem), &d_b);
	err = clSetKernelArg(ko_vadd, 2, sizeof(cl_mem), &d_c);
	err = clSetKernelArg(ko_vadd, 3, sizeof(unsigned int), &count);
	//checkError(err, "Setting kernel arguments");

	GLdouble rtime = wtime();

	// Execute the kernel over the entire range of our 1d input data set
	// letting the OpenCL runtime choose the work group size
}

GLint openClDeviceInfo()
{
	cl_int err;

	// Find the number of OpenCL platforms
	cl_uint num_platforms;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	//checkError(err, "Finding platforms");
	if (num_platforms == 0)
	{
		std::cout << "Found 0 platforms!" << std::endl;
		return EXIT_FAILURE;
	}

	// Create a list of platform IDs
	cl_platform_id platform[20];
	err = clGetPlatformIDs(num_platforms, platform, NULL);
	//checkError(err, "Getting platforms");

	std::cout << "Number of OpenGL platforms: " << num_platforms << std::endl;
	std::cout << "--------------------------" << std::endl;

	// Investigate each platform
	for (GLint i = 0; i < num_platforms; i++)
	{
		cl_char string[10240] = { 0 };
		// Print out the platform name
		err = clGetPlatformInfo(platform[i], CL_PLATFORM_NAME, sizeof(string), &string, NULL);
		//checkError(err, "Getting platform name");
		std::cout << "Platform: " << string << std::endl;

		// Print out the platform vendor
		err = clGetPlatformInfo(platform[i], CL_PLATFORM_VENDOR, sizeof(string), &string, NULL);
		//checkError(err, "Getting platform vendor");
		std::cout << "Vendor: " << string << std::endl;

		// Print out the platform OpenCL version
		err = clGetPlatformInfo(platform[i], CL_PLATFORM_VERSION, sizeof(string), &string, NULL);
		//checkError(err, "Getting platform OpenCL version");
		std::cout << "Version: " << string << std::endl;

		// Count the number of devices in the platform
		cl_uint num_devices;
		err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		//checkError(err, "Finding devices");

		// Get the device IDs
		cl_device_id device[32];
		err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, num_devices, device, NULL);
		//checkError(err, "Getting devices");
		std::cout << "Number of devices: " << num_devices << std::endl;

		// Investigate each device
		for (GLint j = 0; j < num_devices; j++)
		{
			std::cout << "--------------------------" << std::endl;

			// Get device name
			err = clGetDeviceInfo(device[j], CL_DEVICE_NAME, sizeof(string), &string, NULL);
			//checkError(err, "Getting device name");
			std::cout << "\t\tName: " << string << std::endl;

			// Get device OpenCL version
			err = clGetDeviceInfo(device[j], CL_DEVICE_OPENCL_C_VERSION, sizeof(string), &string, NULL);
			//checkError(err, "Getting device OpenCL C version");
			std::cout << "\t\tVersion: " << string << std::endl;

			// Get Max. Compute units
			cl_uint num;
			err = clGetDeviceInfo(device[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &num, NULL);
			//checkError(err, "Getting device max compute units");
			std::cout << "\t\tMax. Compute Units: " << num << std::endl;

			// Get local memory size
			cl_ulong mem_size;
			err = clGetDeviceInfo(device[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
			//checkError(err, "Getting device local memory size");
			std::cout << "\t\tLocal Memory Size: " << mem_size/1024 << "KB" << std::endl;

			// Get global memory size
			err = clGetDeviceInfo(device[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
			//checkError(err, "Getting device global memory size");
			std::cout << "\t\tGlobal Memory Size: " << mem_size / (1024*1024) << "MB" << std::endl;

			// Get maximum buffer alloc. size
			err = clGetDeviceInfo(device[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem_size, NULL);
			//checkError(err, "Getting device max allocation size");
			std::cout << "\t\tMax Alloc Size: " << mem_size / (1024*1024) << "MB" << std::endl;

			// Get work group size information
			size_t size;
			err = clGetDeviceInfo(device[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &size, NULL);
			//checkError(err, "Getting device max work group size");
			std::cout << "\t\tMax Work Group Total Size: " << size << std::endl;

			// Find the maximum dimensions of the work groups
			err = clGetDeviceInfo(device[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &num, NULL);
			//checkError(err, "Getting device max work item dims");
			// Get the max. dimensions of the work groups
			size_t dims[32];
			err = clGetDeviceInfo(device[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(dims), &dims, NULL);
			//checkError(err, "Getting device max work item sizes");
			std::cout << "\t\tMax Work Group Dims: ( ";
			for (size_t k = 0; k < num; k++)
			{
				std::cout << dims[k] << " ";
			}
			std::cout << std::endl << "\t-----------------------" << std::endl;
		}
		std::cout << std::endl << "----------------------------" << std::endl;
	}

	return EXIT_SUCCESS;
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

#pragma once

// Standard libs
#include <vector>
// OpenGL
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW			= -90.0f;
const GLfloat PITCH			=  0.0f;
const GLfloat SPEED			=  3.0f;
const GLfloat SENSITIVITY	=  0.2f;
const GLfloat ZOOM			=  45.0f;

// An abstract camera class that processes input and calculates the 
// corresponding Euler angles, vectors and matrices for use in OpenGL
class Camera
{
public:
	// Camera attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler angles
	GLfloat Yaw;
	GLfloat Pitch;
	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;
	GLfloat groundLevel;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		this->Position = glm::vec3(posX, posY, posZ);
		this->WorldUp = glm::vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler angles and the LookAt matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
	}

	// Processes input received from any keyboard-like input system.  Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	GLvoid ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (direction == FORWARD) { this->Position += this->Front * velocity; }
		if (direction == BACKWARD) { this->Position -= this->Front * velocity; }
		if (direction == LEFT) { this->Position -= this->Right * velocity; }
		if (direction == RIGHT) { this->Position += this->Right * velocity; }
	}

	// Processes input received from a mouse input system.  Expects the offset value in both the x and y direction
	GLvoid ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = GL_TRUE)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f) { this->Pitch = 89.0f; }
			if (this->Pitch < -89.0f) { this->Pitch = -89.0f; }
		}

		// Update Front, Right and Up vectors using the updated Euler angles
		this->updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event.  Only requires input on the vertical wheel-axis
	GLvoid ProcessMouseScroll(GLfloat yoffset)
	{
		if (this->Zoom >= 1.0f && this->Zoom <= 45.0f) { this->Zoom -= yoffset; }
		if (this->Zoom <= 1.0f) { this->Zoom = 1.0f; }
		if (this->Zoom >= 45.0f) { this->Zoom = 45.0f; }
	}

	GLvoid collisionDetect(std::vector<GLfloat> &vertices, GLfloat dTime)
	{
		// Find point on plane above or below camera
		std::vector<GLfloat> dists;
		GLuint idx = 0;
		GLboolean found = GL_FALSE;
		for (GLuint i = 0; i < vertices.size(); i += 24)
		{
			glm::vec3 p1, p2, p3, cam;
			p1 = glm::vec3(vertices[i], 0.0f, vertices[i + 2]);
			p2 = glm::vec3(vertices[i + 8], 0.0f, vertices[i + 10]);
			p3 = glm::vec3(vertices[i + 16], 0.0f, vertices[i + 18]);
			cam = glm::vec3(this->Position.x, 0.0f, this->Position.z);

			// Check if triangle orientation is correct, flip if not
			if (glm::cross(p2 - p1, p3 - p1).y < 0.0f)
			{
				// Flip it
				glm::vec3 temp = p2;
				p2 = p3;
				p3 = temp;
			}
			if (pointInTriag(cam, p1, p2, p3))
			{
				// Found the vertex we care about
				idx = i;
				found = GL_TRUE;
				break;
			}
		}

		GLfloat dist = 0.0f;
		if (found)
		{
			glm::vec3 point = glm::vec3(vertices[idx], vertices[idx + 1], vertices[idx + 2]);
			glm::vec3 norm  = glm::vec3(vertices[idx + 3], vertices[idx + 4], vertices[idx + 5]);
			
			glm::vec3 V = glm::vec3(0.0f, -1.0f, 0.0f);
			GLfloat d = -1.0f * glm::dot(point, norm);
			dist = -1.0f * (glm::dot(this->Position, norm) + d) / glm::dot(V, norm);
			this->groundLevel = this->Position.y - dist;
			//glm::vec3 P = this->Position + dist*V;
			//std::cout << dist << " " << dTime << std::endl;
		}
		else
		{
			// So we aren't sent into space when not hovering over land
			dist = 1.0f;
		}

		if (dist < 0.6f)
		{
			// Push me out of the ground...
			GLfloat push = 10.0f;
			this->Position.y += 10.0f * dTime;
		}
		else if (dist < 0.9)
		{
			GLfloat fall = 10.0f;
			this->Position.y += 0.5f * dTime;
		}
		else if (dist > 1.4f)
		{
			// Fall down
			GLfloat fall = 10.0f;
			this->Position.y -= 10.0f * dTime;
		}
		else if (dist > 1.1f)
		{
			// Fall down
			GLfloat fall = 10.0f;
			this->Position.y -= 0.5f * dTime;
		}
	}

private:
	// Calculates the front vector from the Cameras (updated) Euler angles
	GLvoid updateCameraVectors()
	{
		// Calculate teh new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		this->Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}

	GLboolean pointInTriag(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		if (glm::cross(b - a, p - a).y > 0.0f && 
			glm::cross(c - b, p - b).y > 0.0f &&
			glm::cross(a - c, p - c).y > 0.0f)
		{
			return GL_TRUE;
		}
		else
		{
			return GL_FALSE;
		}
	}
};

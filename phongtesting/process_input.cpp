#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class callback_funcs 
{
	public:
		float fov;
		float lastX;
		float lastY;
		float pitch;
		float yaw;
		float mzoomMax;
		bool firstMouse = true;
		bool mouseLocked = true;
		bool trueUp = false;
		float scrollSens;
		float mouseSens;

		//camera related info
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

		float lastFrame = 0.0f;
		float lastScrollFrame = lastFrame;
		float lastUpToggle = lastFrame;
		float lastMouseToggle = lastFrame;
		float speed;

		callback_funcs(	float FOV = 45.0f,
				float zoomMax = 110.0f,
				float moveSpeed = 10.0f,
				float scrollSensitivity = 2.0f,
				float mouseSensitivity = 0.1f,
				glm::vec3 Pos = glm::vec3(0.0f, 0.0f, 3.0f),
				glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f),
				float lastFrame = 0.0f,
				float Pitch = 0.0f,
				float Yaw = -90.0f,
				bool trueUpEnabled = false,
				bool MouseLocked = true)
		{
			cameraPos = Pos;
			cameraFront = Front;
			cameraUp = Up;
			lastScrollFrame = lastFrame;
			pitch = Pitch;
			yaw = Yaw;
			fov = FOV;
			trueUp = trueUpEnabled;
			mouseLocked = MouseLocked;
			mzoomMax = zoomMax;
			mouseSens = mouseSensitivity;
			scrollSens = scrollSensitivity;
			speed = moveSpeed;
		}

	void init_dims(int width, int height)
	{
		lastX = (float) width/2;
		lastY = (float) height/2;
	}

	void update_fov(float newval)
	{
		fov = newval;
	}

	void process_kb_input(GLFWwindow *window, float deltaTime, float currentTime)
	{
    	float cameraSpeed = static_cast<float>(speed * deltaTime);
    		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && currentTime - lastMouseToggle >= 0.5f){
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			lastMouseToggle = currentTime;
			mouseLocked = false;
			firstMouse = true;
		}
		glm::vec3 right = glm::normalize(glm::cross(glm::normalize(cameraFront), globalUp));
		glm::vec3 up = glm::normalize(glm::cross(right, glm::normalize(cameraFront))); 

		//left right, forward backwards movement
		if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
    		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        		cameraPos += cameraSpeed * cameraFront;
    		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        		cameraPos -= cameraSpeed * cameraFront;
    		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        		cameraPos -= right * cameraSpeed;
    		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        		cameraPos += right * cameraSpeed;
		//toggle between global up down or relative to camera
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && currentTime - lastUpToggle >= 0.5f)
		{
			trueUp = !trueUp;
			lastUpToggle = currentTime;
		}
		//global up down movement
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !trueUp)
			cameraPos.y += cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !trueUp)
			cameraPos.y -= cameraSpeed;
		//up down movement relative to the camera
		//cross product of right and normalized front vectors
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && trueUp)
			cameraPos += up * cameraSpeed;	
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && trueUp)
			cameraPos -= up * cameraSpeed;
	}

	void process_mouse_move(GLFWwindow* window, float xpos, float ypos)
	{
 		if (firstMouse)
		{
        		lastX = xpos;
        		lastY = ypos;
        		firstMouse = false;
    		}
	
	    	float xoffset = xpos - lastX;
	    	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	    	lastX = xpos;
	    	lastY = ypos;
	
	    	float sensitivity = mouseSens; // change this value to your liking
	    	xoffset *= sensitivity;
		yoffset *= sensitivity;
		
		yaw += xoffset;
		pitch += yoffset;
		
		    // make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
		    pitch = 89.0f;
		if (pitch < -89.0f)
		    pitch = -89.0f;
		updateCamVectors(yaw, pitch);
	}

	void process_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset, float prevFrame)
	{
	    lastScrollFrame = prevFrame;
	    fov -= (float)yoffset;
	    if (fov < 1.0f)
	        fov = 1.0f;
	    if (fov > mzoomMax)
	        fov = mzoomMax; 
	}

	void process_mouse_button(GLFWwindow* window, int button, int action, int mods)
	{
		if (!mouseLocked && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	    	{
	    		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	    		mouseLocked = true;
	    		firstMouse = true;
    		}
	}


	void updateCamVectors(float yaw, float pitch)
	{
    		glm::vec3 front;
    		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    		front.y = sin(glm::radians(pitch));
    		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    		cameraFront = glm::normalize(front);
	}
};	

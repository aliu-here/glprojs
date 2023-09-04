#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
//textures
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "include/shader.h"
#include "process_input.cpp"
#include "polygon_funcs.cpp"
#include <vector>


//global values for the callback functions
float fov;

callback_funcs camera(90.0f, 145.0f, 1.5f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (camera.mouseLocked){
		camera.process_mouse_move(window, (float)xposIn, (float)yposIn);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.process_mouse_scroll(window, xoffset, yoffset, lastFrame);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    camera.process_mouse_button(window, button, action, mods);
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	float origfov;
	std::cout << "FOV: ";
	std::cin >> origfov;

	fov = origfov;

	//stupid but we need the camera to be global, so we initialize it with a constant and then redefine it
	camera.update_fov(fov);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	camera.init_dims(mode->width, mode->height);
	
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, mode->width, mode->height);



	//lock cursor to center
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

	int width,height, nrChannels;
	unsigned char *data = stbi_load("resources/rubberroom.jpg", &width, &height, &nrChannels, 0);
	std::cout << "width: " << width << " height: " << height << '\n' << " nrchannels: " << nrChannels << '\n';

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);

	std::cout<< "before cube data\n";

	vector<float> vertices = genCube();

glm::vec3 lightSourceLoc = glm::vec3(1.0f, 0.3f, 1.42f);

	unsigned int cubeVAO, VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
//	glGenBuffers(1, &EBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*4, vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// we only need to bind to the VBO, the container's VBO's data already contains the data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the vertex attribute 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::cout << "main shader\n";
	Shader mainShader("shaders/mainVertex.vert", "shaders/mainFragment.frag");
	std::cout << "light cube shader\n";
	Shader lightCubeShader("shaders/lightVertex.vert", "shaders/lightFragment.frag");

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback); 

	glEnable(GL_DEPTH_TEST);  	
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
        	deltaTime = currentFrame - lastFrame;
        	lastFrame = currentFrame;

		camera.process_kb_input(window, deltaTime, currentFrame);

		if (camera.fov != origfov && currentFrame - camera.lastScrollFrame >= 0.25f)
		{
			camera.update_fov((origfov - camera.fov) / 60 + camera.fov);
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // create transformations
        	glm::mat4 model         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        	glm::mat4 view          = glm::mat4(1.0f);
        	glm::mat4 projection    = glm::mat4(1.0f);

		view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
		projection = glm::perspective(glm::radians(camera.fov), (float)mode->width/mode->height, 0.1f, 100.0f);

		
		mainShader.use();
        	// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        	mainShader.setMat4("projection", projection);
		mainShader.setMat4("model", model);
		mainShader.setMat4("view", view);
		mainShader.setFloat("ratio", 1.0f);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);

		mainShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		mainShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightCubeShader.use();
		glBindVertexArray(lightVAO);
		glm::mat4 move = glm::mat4(1.0f);
		move = glm::translate(move, lightSourceLoc);
		move = glm::scale(move, glm::vec3(0.1f, 0.1f, 0.1f));
		lightCubeShader.setMat4("model", move);
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}




// Declaration (Header):

#include <GL/glew.h>
#include <GLFW/glfw3.h>
//glm
#include <cmath>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>

#include "shader.h"
#include "process_input.cpp"
#include "space_partition/frustum_cull.hpp"

#include "wavefront_loader/loader.hpp"

#include <chrono>

#include <random>

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


void print_point(loader::point in) {
    std::cout << "x: " << in.coord.x << " y: " << in.coord.y << " z: " << in.coord.z << \
        " uv_x: " << in.tex.x << " uv_y: " << in.tex.y << \
        " normal_x: " << in.normal.x << " normal_y: " << in.normal.y << " normal_z: " << in.normal.z << '\n';
}

int main()
{
//	std::string path = getExecPath();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	float origfov = 110;
//	std::cout << "FOV: ";
//	std::cin >> origfov;

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

	glewExperimental = GL_TRUE; 
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Failed to load OpenGL extensions" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, mode->width, mode->height);

    auto start = std::chrono::system_clock::now();
    std::vector<loader::mesh> model = loader::loader("/home/aliu/concave/sphere.obj");
    auto end = std::chrono::system_clock::now();
    auto microsecs = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    glm::vec3 lightSourceLoc = model[0].bounding_box.max;


    std::vector<GLfloat> points;
    std::vector<GLuint> indices;

    long point_count = 0;

    for (loader::mesh part : model) {
        points.insert(points.end(), (float*)&(part.data[0]), (float*)(&(part.data[part.data.size()]))); //ugly but works
        for (auto triangle : part.indices) {
            for (int i=0; i<3; i++) {
                indices.push_back(triangle[i] + point_count);
            }
        }
        point_count += part.data.size();
    }

    std::cout << points.size() << '\n';
    std::cout << indices.size() << '\n';

    int obj_count;
    std::cin >> obj_count;

    octree<int> bbox_vertices({0, 0, 0}, 1000, 2);

    std::vector<glm::vec3> positions(obj_count, glm::vec4(0, 0, 0, 0));

    std::mt19937 engine(0);
    std::uniform_real_distribution dis(-25.0, 25.0);

    std::srand(0);

    for (int i=0; i<obj_count; i++) {
        positions[i] = {dis(engine), dis(engine), dis(engine)};
        for (glm::vec3 bbox_vertex : model[0].bounding_box.get_box_vertices()) {
            bbox_vertices.add_point(bbox_vertex + positions[i], i);
        }
    }

    struct depth_compare
    {
        glm::mat4 m_view;

        depth_compare(glm::mat4 view)
        {
            m_view = view;
        }

        bool operator()(glm::vec3& a, glm::vec3& b) const
        {
            glm::vec3 atransformed = m_view * glm::vec4(a, 1.0f), btransformed = m_view * glm::vec4(b, 1.0f);
            return atransformed.z < btransformed.z;
        }
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    unsigned int instancesVBO;
    glGenBuffers(1, &instancesVBO);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, instancesVBO);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(3, 1);
		
    std::cout << "glGetError: " << glGetError() << '\n';

    //lock cursor to center
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

	std::cout << "main shader\n";
	Shader mainShader("shaders/mainVertex.vert", "shaders/mainFragment.frag");
	std::cout << "light cube shader\n";
	Shader lightCubeShader("shaders/lightVertex.vert", "shaders/lightFragment.frag");

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback); 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE); 
    glEnable(GL_CULL_FACE);


    mainShader.use();

    float avg_fps = NAN;
    int frames_so_far = 0;

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
        	deltaTime = currentFrame - lastFrame;
        	lastFrame = currentFrame;

        if (avg_fps != avg_fps) {
            avg_fps = 1/deltaTime;
            frames_so_far++;
        } else {
            avg_fps = ((avg_fps)*frames_so_far + 1/deltaTime) / (frames_so_far + 1);
            frames_so_far++;
        }

		camera.process_kb_input(window, deltaTime, currentFrame);

		if (camera.fov != origfov && currentFrame - camera.lastScrollFrame >= 0.25f)
		{
			camera.update_fov((origfov - camera.fov) / 60 + camera.fov);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // create transformations
        	glm::mat4 model_mat         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        	glm::mat4 view          = glm::mat4(1.0f);
        	glm::mat4 projection    = glm::mat4(1.0f);

		view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
		projection = glm::perspective(glm::radians(camera.fov), (float)mode->width/mode->height, 0.1f, 1000.0f);

        frustum clip_frustum(projection * view);
        std::vector<int> sure, unsure;
        std::tie(sure, unsure) = get_object_indices(clip_frustum, bbox_vertices, obj_count, 2);
        for (int val : unsure) {
            for (glm::vec3 vertex : model[0].bounding_box.get_box_vertices()) {
                if (clip_frustum.check_point(vertex + positions[val])) {
                    sure.push_back(val);
                    break;
                }
            }
        }

        std::vector<glm::vec3> visible_positions;
        for (int val : sure) {
            visible_positions.push_back(positions[val]);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instancesVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * visible_positions.size(), visible_positions.data(), GL_STATIC_DRAW); 


//        depth_compare comparator = depth_compare(view);
//        std::sort(positions.begin(), positions.end(), comparator);


        // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        mainShader.setMat4("projection", projection);
		mainShader.setMat4("model", model_mat);
		mainShader.setMat4("view", view);
		mainShader.setFloat("ratio", 1.0f);
		mainShader.setVec3("lightPos", lightSourceLoc);
		mainShader.setVec3("lightDir", camera.cameraPos);


//		glEnable(GL_TEXTURE_2D);
//		glBindTexture(GL_TEXTURE_2D, texture);

		mainShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		mainShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, visible_positions.size());
        glBindVertexArray(0);

		lightSourceLoc.x += glm::radians(glm::sin(currentFrame)) * 3;
		lightSourceLoc.y += glm::radians(glm::cos(currentFrame)) * 3;
		lightSourceLoc.z += glm::radians(glm::sin(glm::cos(currentFrame))) * 3;
//		camera.cameraPos = lightSourceLoc;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    std::cout << "average fps: " << avg_fps << '\n';

	glfwTerminate();
	return 0;
}



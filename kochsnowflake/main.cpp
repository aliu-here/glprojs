#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
//textures
#include <stb/stb_image.h>
//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <vector>
#include "include/shader.h"
#include <cmath>
#include <algorithm>

using std::vector;

vector<float> smaller(float a, float b){
	if (a <= b){
		return {a, 0.0f};
	}
	return {b, 0.0f};
}

vector<float> larger(float a, float b){
	if (a <= b){
		return {b, 1.0f};
	}
	return {a, 0.0f};
}

float distance(float dx, float dy){
	return std::sqrt(dx * dx + dy * dy);
}

vector<vector<unsigned int>> updateVAOs(vector<float> points, int width, int height){
	float aspectratio = (float) width / height;
	vector<float> pointsnew;
	pointsnew.insert(pointsnew.end(), points.begin(), points.end());
	for (int i=0; i< (int) pointsnew.size(); i++){
		if (i%3 == 1){
			pointsnew[i] *= aspectratio;
		}
	}
	float vertices[9];
	int trianglenum = pointsnew.size()/9;
	vector<unsigned int> VAO(trianglenum), VBO(trianglenum);
//	unsigned int VAO[trianglenum], VBO[trianglenum];
	glGenVertexArrays(trianglenum, VAO.data());
	glGenBuffers(trianglenum, VBO.data());

	std::cerr << "before gen vao and vbo\n";
	for (int i=0; i<trianglenum; i++){
		//std::cerr << "before make the triangle\n";
		for (int j=0; j<9; j++){
			vertices[j] = pointsnew[i * 9 + j];
		}
		//std::cerr << i << "\n";
		//std::cerr << "before binding vao and vbo\n";
		glBindVertexArray(VAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*9, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	vector<vector<unsigned int>> output = {VAO, VBO};
	return output;
}

//honestly idk what i was doing
vector<float> createObjs(float edge[2][3], int depth){
	vector<float> points;
	if (depth < 0){
		return points;
	}
	//get the x and y values sorted for ease of use later
	vector<float> leftx = smaller(edge[0][0], edge[1][0]);
	vector<float> rightx = larger(edge[0][0], edge[1][0]);
	int leftpoint = leftx[1];
	int rightpoint = rightx[1];
	//delta of x and y
	std::cerr << "leftmost point: " << leftpoint << '\n';
	float dx = rightx[0] - leftx[0];
	float dy = edge[rightpoint][1] - edge[leftpoint][1];
	//debugging??? this doesnt even happen normally for some reason
	if (leftpoint == 1){
	std::cerr << "leftpoint: " << leftpoint << "\nrightpoint: " << rightpoint << "\ndx: " <<  dx << "\n leftedge: " << edge[leftpoint][1] << "\n rightedge: " << edge[rightpoint][1] << '\n';
	std::cerr << "dy: " << dy << '\n';
	}
	//distance of edge & slope
	float length = distance(dx, dy);

	std::cerr << length << '\n';
	//height of the triangle
	float height = (length / 6) * sqrt(3);
	//slope of the perpendicular line to the edge
	float perpslope = -(dx)/(dy);
	std::cout << "slope: " << perpslope << '\n';
	//for the case where the slope is negative to keep the triangle from being drawn inwards
	if ( perpslope > -1 && perpslope < 0 && leftpoint == 0){
		height *= -1;
	}
	float mid[2] = {leftx[0] + dx / 2, edge[leftpoint][1] + dy / 2};
	std::cerr <<"midx: " << mid[0] << "\nmidy: " << mid[1] << '\n';

	//now for the thirds
	float firstthird[] = {leftx[0] + dx / 3, edge[leftpoint][1] + dy/3, 0.0f};
	float secthird[] = {leftx[0] + 2 * dx / 3, edge[leftpoint][1] + 2 * dy/3, 0.0f};
	float thirdpoint[3];
	thirdpoint[2] = 0.0f;

	//third point
	if (dy != 0){
		thirdpoint[0] = mid[0] + height * (float) (1/sqrt(1 + perpslope*perpslope));
		thirdpoint[1] = mid[1] + height * (float) (perpslope/sqrt(1 + perpslope*perpslope));
	} else {
		thirdpoint[0] = mid[0];
		thirdpoint[1] = mid[1] + height;
	}
	points.insert(points.end(), thirdpoint, thirdpoint + 3);
	points.insert(points.end(), firstthird, firstthird + 3);
	points.insert(points.end(), secthird, secthird + 3);

	for (int i=0; i<(int)points.size(); i++){
		std::cerr << points[i] << ' ' << depth << '\n';
	}

	//first recursion, leftmost
	float newedge[2][3] = {{edge[0][0], edge[0][1], edge[0][2]},
			       {firstthird[0], firstthird[1], firstthird[2]}};
	vector<float> recurse1 = createObjs(newedge, depth - 1);
	
	//second recursion, rightmost
	for (int i=0; i<3; i++){
		newedge[0][i] = secthird[i];
	}
	for (int i=0; i<3; i++){
		newedge[1][i] = edge[1][i];
	}
	vector<float> recurse2 = createObjs(newedge, depth - 1);

	//third & fourth recursion; on the new triangle 
	for (int i=0; i<3; i++){
		newedge[0][i] = firstthird[i];
	}
	for (int i=0; i<3; i++){
		newedge[1][i] = thirdpoint[i];
	}
	vector<float> recurse3 = createObjs(newedge, depth - 1);

	for (int i=0; i<3; i++){
		newedge[0][i] = thirdpoint[i];
	}
	for (int i=0; i<3; i++){
		newedge[1][i] = secthird[i];
	}
	vector<float> recurse4 = createObjs(newedge, depth - 1);

	points.insert(points.end(), recurse1.begin(), recurse1.end());
	points.insert(points.end(), recurse2.begin(), recurse2.end());
	points.insert(points.end(), recurse3.begin(), recurse3.end());
	points.insert(points.end(), recurse4.begin(), recurse4.end());

	return points;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

float vecsum(vector<float> vec){
	float out=0;
	for (int i=0; i< (int) vec.size(); i++){
		out += vec[i];
	}
	return out;
}

int main()
{
	int width, height;
	std::cout << "Input window width: ";
	std::cin >> width;
	std::cout << "Input window height: ";
	std::cin >> height;
	float aspectratio = (float)width/height;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
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

	glViewport(0, 0, 800, 600);

	float edge[2][3] = {
		{-0.5625f/ (float) sqrt(3), 0.5625f, 0.0f},
		{0.5625f/ (float) sqrt(3), 0.5625f, 0.0f}
	};

	std::cerr << "before create objects\n";
	vector<float> points = createObjs(edge, 3);
	//unchanged copy for updating the objects
	vector<float> pointscopy = points;

	std::cerr << "after create objects\n";
	std::cerr << points.size() << '\n';

	for (int i=0; i<2; i++){
		for (int j=0; j<3; j++){
			points.push_back(edge[i][j]);
		}
	}
	for (int i=0; i<3; i++){
		points.push_back(0.0f);
	}
	for (int i=0; i<5; i++){
		points.insert(points.end(), pointscopy.begin(), pointscopy.end());
	}
	for (int i=0; i<(int)points.size(); i++){
		if (i%3 == 1){
			points[i] *= aspectratio;
		}
	}
	pointscopy.clear();
	pointscopy.insert(pointscopy.end(), points.begin(), points.end());

	float vertices[9];
	int trianglenum = points.size()/9;
	

	vector<unsigned int> VAO(trianglenum), VBO(trianglenum);
//	unsigned int VAO[trianglenum], VBO[trianglenum];
	glGenVertexArrays(trianglenum, VAO.data());
	glGenBuffers(trianglenum, VBO.data());

	std::cerr << "before gen vao and vbo\n";
	//create vaos and vbos
	for (int i=0; i<trianglenum; i++){
		//std::cerr << "before make the triangle\n";
		for (int j=0; j<9; j++){
			vertices[j] = points[i * 9 + j];
		}
		//std::cerr << i << "\n";
		//std::cerr << "before binding vao and vbo\n";
		glBindVertexArray(VAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*9, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	std::cerr << "before shaders\n";

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::cerr << "before compile shaders\n";
	Shader ourShader("shaders/vertex.glsl", "shaders/fragment.glsl");
	std::cerr << "after shaders\n";

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	for (int i=0; i<(int) points.size(); i++){
//		std::cerr << points[i] << '\n';
	}
	glm::mat4 trans = glm::mat4(1.0f);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* videomode = glfwGetVideoMode(primary);
		if (videomode -> width != width || videomode -> height != height){
			vector<vector<unsigned int>> newobjs = updateVAOs(pointscopy, videomode -> width, videomode -> height);
			VAO.clear();
			VAO.insert(VAO.end(), newobjs[0].begin(), newobjs[0].end());
			VBO.clear();
			VBO.insert(VBO.end(), newobjs[1].begin(), newobjs[1].end());
			width = videomode -> width;
			height = videomode -> height;
		}

		ourShader.use();
	
		for (int i=0; i<6; i++){
			trans = glm::rotate(trans, glm::radians(60.0f), glm::vec3(0.0, 0.0, 1.0));
			unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
			for (int i=0; i<trianglenum; i++){
				glBindVertexArray(VAO[i]);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}



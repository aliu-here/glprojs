#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

using std::vector;

glm::vec3 normal(vector<float> points)
{
	glm::vec3 point1 = glm::vec3(points[0], points[1], points[2]);
	glm::vec3 point2 = glm::vec3(points[3], points[4], points[5]);
	glm::vec3 point3 = glm::vec3(points[6], points[7], points[8]);
	return glm::normalize(glm::cross(point2-point1, point2-point3));
}

vector<float> slice(vector<float> in, int start, int end)
{
	vector<float> out;
	for (int i=start; i<end; i++)
	{
		out.push_back(in[i]);
	}
	return out;
}

vector<float> genCube()
{
	vector<float> triangle;
	vector<glm::vec3> normals;
	vector<float> points = 	{-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f};
	vector<float> out; 
  
    for (int i=0; i<(int)(points.size()/5)/3; i++)
    {
	    for (int j=0; j<15; j++)
	    {
		    if (j%5 <= 2){
		    	triangle.push_back(points[i*15 + j]);
		    }
	    }
	    glm::vec3 trianglenormal = normal(triangle);
	    normals.push_back(trianglenormal);
    }
    for (int i=0; i<(int)(points.size()/5) / 3; i++)
    {
	    for (int j=0; j<3; j++)
	    {
		    vector<float> subvector = slice(points, i*15 + j*5, i*15 + j*5 + 5);
		    out.insert(out.end(), subvector.begin(), subvector.end());
		    out.push_back(normals[i][0]);
		    out.push_back(normals[i][1]);
		    out.push_back(normals[i][2]);
	    }
    }
    return out;
}


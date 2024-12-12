#include <functional>
#include <vector>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>

class Point 
{
	public:
	glm::vec3 normal = glm::vec3(0, 0, 0);
	glm::vec3 pos;
	Point (glm::vec3 in)
	{
		pos = in;
	}
	Point(float a=0, float b=0, float c=0)
	{
		pos = glm::vec3(a, b, c);
	}
};

class Triangle
{
	public:
	Point v[3];
	Triangle(Point point1, Point point2, Point point3)
	{
		v[0] = point1;
		v[1] = point2;
		v[2] = point3;
//		sortCounterClockwise();
	};
	Triangle (float points[9])
	{
		for (int i=0; i<3; i++)
		{
			v[i] = Point(points[i*3], points[i*3+1], points[i*3+2]);
		}
//		sortCounterClockwise();
	}

/*	private:
	void sortCounterClockwise()
	{
		std::vector<float> angles;
		std::vector<glm::vec3> points;
		std::vector<glm::vec3> tocenter;
		for (int j=0; j<3; j++)
		{
			points.push_back(v[j].pos);
		}
		glm::vec3 center = avg(points);
		for (int j=0; j<3; j++)
		{
			tocenter.push_back(glm::normalize(center - v[j].pos));
		}
		for (int j=0; j<2; j++)
		{
			float temp = glm::dot(tocenter[0], tocenter[1+j])/(glm::length(tocenter[0]) * glm::length(tocenter[1+j]));
			temp = glm::degrees(glm::acos(temp));
			angles.push_back(temp);
		}
		std::cout << angles[0] << ' ' << angles[1] << '\n';
		if (angles[0] < angles[1])
		{
			v[0] = points[0];
			v[1] = points[2];
			v[2] = points[1];
		}
	};
*/
};

class Mesh
{
	public:
	std::vector<Triangle> triangles;
	int faceNum;
	Mesh (std::vector<Triangle> faces, int facesNum)
	{
		triangles = faces;
		faceNum = facesNum;
	};
	void calcNorms()
	{
		std::vector<glm::vec3> points;
		//wipe the normals just in case they were previously put
		for (int i=0; i<faceNum; i++)
		{
			for (int j=0; j<3; j++)
			{
				triangles[i].v[j].normal = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}
		for (int i=0; i<faceNum; i++)
		{
			glm::vec3 ia = triangles[i].v[0].pos;
			glm::vec3 ib = triangles[i].v[1].pos;
			glm::vec3 ic = triangles[i].v[2].pos;

			points.push_back(ia); points.push_back(ib); points.push_back(ic);

			glm::vec3 e1 = ia - ib;
			glm::vec3 e2 = ic - ib;
			glm::vec3 norm = glm::cross(e1, e2);

			for (int j=0; j<3; j++)
			{
				triangles[i].v[j].normal += norm;
			}

		}
		for (int i=0; i<faceNum; i++)
		{
			for (int j=0; j<3; j++)
			{
				triangles[i].v[j].normal = normalize(triangles[i].v[j].normal);
				std::cout << triangles[i].v[j].normal[0] << ' ' << triangles[i].v[j].normal[1] << ' ' << triangles[i].v[j].normal[2] << '\n';
			}
		}

		glm::vec3 center = avg(points);

		for (int i=0; i<faceNum; i++)
		{
			for (int j=0; j<3; j++)
			{

			if (glm::dot(triangles[i].v[j].pos - center, triangles[i].v[j].normal) > 0)
					triangles[i].v[j].normal *= -1;
			triangles[i].v[j].normal *= -1;
			}
		}
	};
	
	glm::vec3 avg(std::vector<glm::vec3> points)
	{
		glm::vec3 out = glm::vec3(0, 0, 0);
		for (int i=0; i<points.size(); i++)
		{
			out += points[i];
		}
		out /= points.size();
		return out;
	}

	std::vector<float> exportPointData()
	{
		std::vector<float> out;
		for (int i=0; i<faceNum; i++)
		{
			for (int j=0; j<3; j++)
			{
				for (int k=0; k<3; k++)
				{
					out.push_back(triangles[i].v[j].pos[k]);
				}
				for (int k=0; k<3; k++)
				{
					out.push_back(triangles[i].v[j].normal[k]);
				}
			}
		}
		return out;
	}
};

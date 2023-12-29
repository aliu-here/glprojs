#include <vector>
#include <string>
#include <glm/glm.hpp>

#ifndef MESH_CLASS
#define MESH_CLASS

namespace loader {
typedef struct point_data
{
	float data[8];
	bool use_tex, use_normals;
} point_data;

typedef struct Triangle
{
	point_data points[3];
}triangle;

typedef struct material
{
	std::string texture_path, name;
	glm::vec3 ambient, diffuse, specular;
	float specular_exp;
}material;


class mesh
{
	public:
		std::vector<triangle> data;
		std::string group_name;
		material used_mtl;
		std::vector<float> export_data()
		{
			std::vector<float> output;
			for (triangle it: data)
			{
				for (int i=0; i<3; i++){ output.insert(output.end(), &it.points[i].data[0], &it.points[i].data[7]); }
			}
			return output;
		}
};
}

#endif

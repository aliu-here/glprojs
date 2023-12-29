#include <unordered_map>
#include "mesh.hpp"

#ifndef OBJ_LOADER
#define OBJ_LOADER

namespace loader
{
	//loads material file into std::unordered map; keys are the names of the material 
	std::unordered_map<std::string, material> load_mtl(const std::string& path);
	//generates std::vector<triangle> from f line in .obj file input
	std::vector<triangle> generate_triangles(const std::vector<std::string>& data, int line_num, const std::vector<std::vector<float>>& vert_data, const std::vector<std::vector<float>>& uv_coord_data, const std::vector<std::vector<float>>& normal_data, int vertex_offset, int uv_offset, int normal_offset);
	//loads .obj file from path, returns std::vector<mesh>; mesh is an std::vector<triangle>
	std::vector<mesh> loader(const std::string& path);
}
#endif

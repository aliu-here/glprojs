#include "mesh.hpp"

#ifndef OBJ_LOADER
#define OBJ_LOADER

namespace loader
{
	//loads .obj file from path, returns std::vector<mesh>; mesh is an std::vector<triangle>
	std::vector<mesh> loader(const std::string& path);
}
#endif

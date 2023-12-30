#ifndef LOADER_HPP
#define LOADER_HPP

#include "mesh.hpp"

namespace loader
{
    //loads .obj file from path, returns std::vector<mesh>; mesh is an std::vector<triangle>
    std::vector<mesh> loader(const std::string& path);
}
#endif

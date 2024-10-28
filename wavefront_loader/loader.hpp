#ifndef LOADER_HPP
#define LOADER_HPP

#include "mesh.hpp"

enum windings {
    CW=false,
    CCW=true
};



namespace loader
{
    /**
     * Loads a .obj wavefront file, specified as an std::string which is the path to the file;
     * there is an option to use mulithreading to triangulate the file, which is off by default
     * */
    std::vector<mesh> loader(const std::string& path, bool usemt=false, int threadcount=0);
}
#endif

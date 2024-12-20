#include <fstream>
#include <string>
#include <stddef.h>
#include <glm/glm.hpp>
#include <cstring>
#include "mesh.hpp"

/*
 * file description:
 * header: char[4] "BOBJ"
 * endianness flag: char either "B" or "L"
 * mesh count: uint32
 * per mesh:
 * mesh name size (in bytes): uint32
 * mesh name: char[mesh name size]
 * points vector size (in bytes): uint32
 * 
 * */

namespace loader {

    std::string ser_obj(void *objptr, unsigned int size) {
        return std::string(reinterpret_cast<char*>(objptr), size);
    }

    std::string get_file_contents(std::string filename)
    {
      std::ifstream in(filename, std::ios::in | std::ios::binary);
      if (in)
      {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
      }
      throw(errno);
    }

    bool test_endianness() {
        int test_num = 0x41424344;
        char *to_char;
        to_char = reinterpret_cast<char*>(&test_num);
        return (std::string(to_char, sizeof(int)) == "DCBA"); //returns true if little-endian, false if big-endian
    }

    void save_model(model model, std::string savepath) 
    {
        std::ofstream out(savepath + ".bobj");
        out << "BOBJ";
        glm::uint32_t temp = model.size();
        out << ser_obj(&temp, sizeof(temp));
        for (mesh mesh : model) {
            temp = mesh.group_name.size();
            out << ser_obj(&temp, sizeof(temp)); 
            out << mesh.group_name;
            temp = mesh.data.size() * sizeof(point);
            out << ser_obj(&temp, sizeof(temp));
            for (point point : mesh.data) { //print point data
                out << ser_obj(&point, sizeof(point));
            }
            temp = mesh.indices.size() * 3 * sizeof(uint32_t);
            out << ser_obj(&temp, sizeof(int));
            for (std::array<unsigned int, 3> tri : mesh.indices) {
                out << ser_obj(&tri, sizeof(tri));
            }

            temp = mesh.used_mtl.name.size();
            out << ser_obj(&temp, sizeof(int));
            out << mesh.used_mtl.name;
            out << ser_obj(&mesh.used_mtl.ambient, sizeof(glm::vec3));
            out << ser_obj(&mesh.used_mtl.diffuse, sizeof(glm::vec3));
            out << ser_obj(&mesh.used_mtl.specular, sizeof(glm::vec3));
            out << ser_obj(&mesh.used_mtl.specular_exp, sizeof(glm::float32_t));
        }
    }
    model load_model(std::string path)
    {
        std::string file = get_file_contents(path + ".bobj");
    }
}

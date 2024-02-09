#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <tuple>

const float FLT_INF = std::numeric_limits<float>::infinity();

namespace loader {

struct point 
{
    glm::vec3 coord = glm::vec3(0, 0, 0);
    glm::vec2 tex = glm::vec2(0, 0);
    glm::vec3 normal = glm::vec3(0, 0, 0);
};
struct box { glm::vec3 min = glm::vec3(0, 0, 0), max = glm::vec3(FLT_INF, FLT_INF, FLT_INF); };

struct material
{
    std::string texture_path, name;
    glm::vec3 ambient = glm::vec3(0, 0, 0), diffuse=glm::vec3(0, 0, 0), specular=glm::vec3(0, 0, 0);
    float specular_exp=0;
};

struct directed_angles
{
    std::tuple<float, glm::vec3> outer = {0, glm::vec3(0, 0, 0)}, inner = {0, glm::vec3(0, 0, 0)};
};

struct mesh
{
    std::vector<point> data;
    std::vector<std::array<int, 3>> indices;
    std::string group_name;
    material used_mtl;
    box bounding_box;
    const std::vector<float> export_data()
    {
        std::vector<float> output;
        for (point it: data)
        {
            output.insert(output.end(), &(it.coord[0]), &(it.coord[3]));
            output.insert(output.end(), &(it.tex[0]), &(it.tex[3]));
            output.insert(output.end(), &(it.normal[0]), &(it.normal[3]));
        }
        return output;
    }
};

typedef std::vector<mesh> model;
}


#endif

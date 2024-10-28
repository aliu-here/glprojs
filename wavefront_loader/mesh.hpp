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
    glm::vec3 coord = glm::vec3(0.0, 0.0, 0.0);
    glm::vec2 tex = glm::vec2(0.0, 0.0);
    glm::vec3 normal = glm::vec3(0.0, 0.0, 0.0);
};

union point_to_float
{
    point vec_form_data;
    float raw_data[8];
};

struct box { glm::vec3 min = glm::vec3(0, 0, 0), max = glm::vec3(0.0, 0.0, 0.0); };

struct material
{
    std::string texture_path, name;
    glm::vec3 ambient = glm::vec3(0.56, 0.56, 0.56), diffuse=glm::vec3(0.2, 0.2, 0.2), specular=glm::vec3(0.8, 0.8, 0.8); //light grey color, like blender
    float specular_exp=0.5;
    
    material() {}
};

struct directed_angles
{
    std::tuple<float, glm::vec3> outer = {0, glm::vec3(0, 0, 0)}, inner = {0, glm::vec3(0, 0, 0)};
};

struct mesh
{
    std::vector<point> data;
    std::vector<std::array<unsigned int, 3>> indices;
    std::string group_name;
    material used_mtl;
    box bounding_box;

    mesh() {}

    std::vector<float> export_data()
    {
        std::vector<float> output;
        for (point it: data)
        {
            point_to_float temp = {it};
            output.insert(output.end(), &(temp.raw_data[0]), &(temp.raw_data[7]));
        }
        return output;
    }
};

typedef std::vector<mesh> model;
}


#endif

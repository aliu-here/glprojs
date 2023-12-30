#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <limits>

const float FLT_INF = std::numeric_limits<float>::infinity();

namespace loader {

struct point { float point_data[8]; };
struct box { glm::vec3 coords[2] = {glm::vec3(0, 0, 0), glm::vec3(FLT_INF, FLT_INF, FLT_INF)}; };
struct triangle { point points[3]; };

struct material
{
    std::string texture_path, name;
    glm::vec3 ambient, diffuse, specular;
    float specular_exp;
};


struct mesh
{
    std::vector<triangle> data;
    std::string group_name;
    material used_mtl;
    box bounding_box;
    std::vector<float> export_data()
    {
        std::vector<float> output;
            for (triangle it: data)
            {
                for (int i=0; i<3; i++){ output.insert(output.end(), &it.points[i].point_data[0], &it.points[i].point_data[7]); }
            }
            return output;
        }
};
}

#endif

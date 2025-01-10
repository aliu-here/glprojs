#pragma once

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <filesystem>

const float FLT_INF = std::numeric_limits<float>::infinity();

namespace loader {

    struct point 
    {
        glm::vec3 coord = glm::vec3(0.0, 0.0, 0.0);
        glm::vec2 tex = glm::vec2(0.0, 0.0);
        glm::vec3 normal = glm::vec3(0.0, 0.0, 0.0);
    };

    struct box { glm::vec3 min = glm::vec3(0, 0, 0), max = glm::vec3(0.0, 0.0, 0.0); };

    struct material
    {
        std::string texture_path, name;
        glm::vec3 ambient = glm::vec3(0.56, 0.56, 0.56), diffuse=glm::vec3(0.2, 0.2, 0.2), specular=glm::vec3(0.8, 0.8, 0.8); //light grey color, like blender
        glm::float32_t specular_exp=0.5;
        
        material() {}
    };

    struct mesh
    {
        std::vector<float> export_points()
        {
            std::vector<float> output;
            for (point it: data)
            {
                output.push_back(it.coord.x);
                output.push_back(it.coord.y);
                output.push_back(it.coord.z);

                output.push_back(it.tex.x);
                output.push_back(it.tex.y);

                output.push_back(it.normal.x);
                output.push_back(it.normal.y);
                output.push_back(it.normal.z);
            }
            return output;
        }
        std::vector<unsigned int> export_indices(int offset=0)
        {
            std::vector<unsigned int> output;
            for (std::array<unsigned int, 3> tri : indices) {
                for (unsigned int val : tri)
                    output.push_back(val + offset);
            }
            return output;
        }

        std::vector<point> data;
        std::vector<std::array<uint32_t, 3>> indices;
        std::string group_name;
        material used_mtl;
        box bounding_box;

        void save_as(std::filesystem::path filename) {
        }
    };

    using model = std::vector<mesh>;
}

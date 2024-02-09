#include <array>
#include <vector>
#include <string>
#include <unordered_map>

#include "mesh.hpp"

namespace loader
{ 
std::tuple<std::vector<point>, std::vector<std::array<int, 3>>> process_faces(const std::vector<std::string>& faces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, const int line_num, const int coord_offset, const int tex_offset, const int normal_offset); //oh god this is terrible
    void print_vec3(glm::vec3 in);
}

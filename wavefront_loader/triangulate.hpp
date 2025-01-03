#include <array>
#include <vector>
#include <string>
#include <memory>

#include "mesh.hpp"

namespace loader
{ 
     std::tuple<std::unique_ptr<std::vector<point>>, std::unique_ptr<std::vector<std::array<unsigned int, 3>>>>  process_faces(const std::vector<std::string>& faces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals);
    void print_vec3(glm::vec3 in);
}

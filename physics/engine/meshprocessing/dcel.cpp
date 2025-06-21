#include <vector>
#include <glm/glm.hpp>
#include <array>
#include <string>
#include <unordered_map>

namespace physicsengine::meshprocessing {
    using halfedge_pointer = int;
    using face_pointer = int;
    using vertex_pointer = int;
    struct dcel_halfedge {
        face_pointer face_index = -1;
        vertex_pointer origin_vertex_index = -1; 
        halfedge_pointer next_edge = -1, prev_edge = -1, twin = -1;
    };
    struct dcel_face {
        halfedge_pointer bounding_halfedge = -1;
    };
    struct dcel_vertex {
        glm::vec3 coord;
        halfedge_pointer connected_halfedge = -1;
    };

    struct dcel {
        private:
            std::vector<dcel_vertex> vertices;
            std::unordered_map<std::string, int> vertex_indices;

            std::string vec3_to_string(glm::vec3 vec)
            {
                return std::string(reinterpret_cast<char*>(&vec), sizeof(vec));
            }

        public:
            std::vector<dcel_halfedge> halfedges;
            std::vector<dcel_face> faces;

            dcel(const std::vector<glm::vec3>& verts, const std::vector<std::array<int, 3>>& indices) {
                for (int i=0; i<verts.size(); i++) {
                    vertex_indices[vec3_to_string(verts[i])] = i;
                    dcel_vertex tempvert;
                    tempvert.coord = verts[i];
                    vertices.push_back(tempvert);
                }

                for (std::array<int, 3> tri : indices) {
                    add_face({verts[tri[0]], verts[tri[1]], verts[tri[2]]});
                }
            }

            void add_face(const std::vector<glm::vec3>& verts) {
                for (int i=0; i<verts.size(); i++) {
                    int next = (i + 1) % verts.size();
                }
            }
    };
}

#include <vector>
#include <glm/glm.hpp>

namespace physicsengine::meshprocessing {
    struct dcel_edge;
    struct dcel_face;

    struct dcel {
        private:
            std::vector<glm::vec3> vertices;
        public:
            std::vector<dcel_edge> edge;
            std::vector<dcel_face> faces;

            dcel(std::vector<glm::vec3> verts);
    };
}

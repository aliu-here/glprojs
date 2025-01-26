#include "../wavefront_loader/loader.hpp"
#include "frustum_cull.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>

#include <chrono>

std::vector<int> naive(loader::box box, std::array<glm::vec3, 100000>& positions, frustum f)
{
    int counter = 0;
    std::vector<int> naive_result;
    std::vector<glm::vec3> vertices = box.get_box_vertices();
    for (glm::vec3 displacement : positions) {
        for (glm::vec3 vertex : vertices) {
            if (f.check_point(vertex + displacement)) {
                naive_result.push_back(counter);
                break;
            }
        }
        counter++;
    }
    return naive_result;
}

int main()
{
    loader::model model = loader::loader("/home/aliu/concave/sphere.obj");
    
    loader::box bbox = model[0].bounding_box;

    std::array<glm::vec3, 100000> positions;

    std::mt19937 engine(0);
    std::uniform_real_distribution dis(-100.0, 100.0);

    std::srand(0);
    for (int i=0; i<100000; i++) {
        positions[i] = {dis(engine), dis(engine), dis(engine)};
    }


    octree<int> bbox_octree = octree<int>({0, 0, 0}, 205, 5);
    int counter = 0;
    for (glm::vec3 displacement : positions) {
        for (glm::vec3 vertex : bbox.get_box_vertices())
            bbox_octree.add_point(vertex + displacement, counter);
        counter++;
    }

    glm::mat4 proj_mat = glm::perspective(glm::radians(110.f), 1920.f/1080.f, 1.f, 1000.f);

    frustum f(proj_mat);

    auto begin = std::chrono::system_clock::now();
    std::vector<int> indices = get_object_indices(f, bbox_octree, 100000);
    auto end = std::chrono::system_clock::now();
    std::cout << "octree: " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "us" << '\n';

    begin = std::chrono::system_clock::now();
    std::vector<int> naive_result = naive(model[0].bounding_box, positions, f);
    end = std::chrono::system_clock::now();
    std::cout << "naive: " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "us" << '\n'; 

    bool func_correct = (naive_result.size() == indices.size());
    std::cout << naive_result.size() << ' ' << indices.size() << '\n';

    if (func_correct) {
        for (int i=0; i<naive_result.size(); i++) {
//            std::cout << naive_result[i] << ' ' << indices[i] << '\n';
            if (naive_result[i] != indices[i]) {
                func_correct = false;
                break;
            }
        }
    }

    std::cout << "naive matches octree? " << func_correct << '\n';
}

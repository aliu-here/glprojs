#include "../wavefront_loader/mesh.hpp"
#include "frustum_cull.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>

#include <chrono>
#include <algorithm>

int naive_check_count = 0;
__attribute__((noinline)) std::vector<int> naive(loader::box box, std::vector<glm::vec3>& positions, frustum f)
{
    int counter = 0;
    std::vector<int> naive_result;
    const std::vector<glm::vec3> vertices = box.get_box_vertices();
    for (glm::vec3 displacement : positions) {
        if (f.check_sphere(displacement, glm::length(box.max -  (box.max + box.min) * 0.5f)))
            naive_result.push_back(counter);
/*        for (glm::vec3 vertex : vertices) {
            naive_check_count++;
            if (f.check_point(vertex + displacement)) {
                naive_result.push_back(counter);
                break;
            }
        }*/
        counter++;
    }
    return naive_result;
}

__attribute__((noinline)) std::vector<int> frustum_cull(octree<int> bbox_octree, loader::box box, std::vector<glm::vec3>& positions, frustum f, int subdiv_level, int obj_count)
{
    const std::vector<glm::vec3> vertices = box.get_box_vertices();
    std::vector<int> sure, unsure;
    std::tie(sure, unsure) = get_object_indices(f, bbox_octree, obj_count, subdiv_level);
    
    for (int val : unsure) {
        if (f.check_sphere(positions[val], glm::length(box.max - (box.max + box.min) * 0.5f)))
            sure.push_back(val);
/*        for (glm::vec3 vertex : vertices) {
            if (f.check_point(vertex + positions[val])) {
                sure.push_back(val);
                break;
            }
        }*/
    }
    return sure;
}

int main()
{
    int obj_count = 10000;

    int subdiv_level;
    std::cin >> subdiv_level;
 
    loader::box bbox = {glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1)};
    std::vector<glm::vec3> positions(obj_count);

    std::mt19937 engine(0);
    std::uniform_real_distribution dis(0.0, 200.0);
    std::uniform_real_distribution dis2(-100.0, 100.0);

    std::srand(0);
    for (int i=0; i<obj_count; i++) {
        positions[i] = {dis2(engine), dis2(engine), dis(engine)};
    }

    std::cout << "building octree...\n";
    octree<int> bbox_octree({0, 0, 100}, 205, subdiv_level);
    int counter = 0;
    const std::vector<glm::vec3> vertices = bbox.get_box_vertices();
    for (glm::vec3 displacement : positions) {
        for (glm::vec3 vertex : vertices)
            bbox_octree.add_point(vertex + displacement, counter);
        counter++;
    }
    std::cout << "octree built\n";

    glm::mat4 proj_mat = glm::perspective(glm::radians(110.f), 1920.f/1080.f, 1.f, 1000.f);

    frustum f(proj_mat);

    auto begin = std::chrono::system_clock::now();
    std::vector<int> sure = frustum_cull(bbox_octree, bbox, positions, f, subdiv_level, obj_count);
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "us for octree" << '\n';

    begin = std::chrono::system_clock::now();
    std::vector<int> naive_result = naive(bbox, positions, f);
    end = std::chrono::system_clock::now();
    std::cout << naive_check_count << " checks for naive\n";
    std::cout << "naive: " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "us" << '\n'; 

    bool func_correct = (naive_result.size() == sure.size());
    std::cout << naive_result.size() << ' ' << sure.size() << '\n';

    if (func_correct) {
        std::sort(sure.begin(), sure.end());
        for (int i=0; i<naive_result.size(); i++) {
//            std::cout << naive_result[i] << ' ' << indices[i] << '\n';
            if (naive_result[i] != sure[i]) {
                func_correct = false;
                break;
            }
        }
    }

    std::cout << "naive matches octree? " << func_correct << '\n';
}

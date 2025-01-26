#include "ll_octree.hpp"
#include <iostream>

int main()
{
    octree<int> tree = octree<int>(glm::vec3(0, 0, 0), 100, 1);
    int counter = 0;
    for (int i=-5; i < 11; i++) {
        for (int j=-5; j < 11; j++) {
            for (int k=-5; k< 11; k++) {
                tree.add_point(glm::vec3(i, j, k), counter);
                counter++;
            }
        }
    }

    octree_node root = tree.get_root();
    std::vector<octree_node> nodes = tree.get_children_of_node(root);

    for (octree_node node : nodes) {
        for (int val : tree.get_leaf_values(node)) {
            std::cout << val << ' ';
        }  
    }

    std::cout << '\n';
}

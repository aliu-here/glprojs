#include "octree.hpp"
#include <iostream>

int main()
{
    octree<int> tree = octree<int>();
    int counter = 0;
    for (int i=-5; i < 6; i++) {
        for (int j=-5; j < 6; j++) {
            for (int k=-5; k< 6; k++) {
                tree.add_point(glm::vec3(i, j, k), counter);
                counter++;
            }
        }
    }

    octree_node root = tree.get_node_at_idx(0);
    octree_node octant_0 = tree.get_node_at_idx(root.children[0][0][0]);

    for (int val : tree.get_leaf_values(octant_0)) {
        std::cout << val << ' ';
    }

    std::cout << '\n';
}

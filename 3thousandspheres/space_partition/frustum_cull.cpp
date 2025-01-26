#include "octree.hpp"
#include "frustum.hpp"

#include <vector>
#include <cmath>
#include <iostream>

const float SQRT2 = std::sqrt(2);

int check_count = 0;

enum frustum_result {
    FULLY_INSIDE,
    PARTIALLY_INSIDE,
    OUTSIDE
};

frustum_result frustum_intersects_octree_subdiv_count(octree_node node, frustum frustum)
{
    check_count += 8;
    int count = 0;
    for (int add_x = -1; add_x <= 1; add_x+=2) {
        for (int add_y = -1; add_y <= 1; add_y+=2) {
            for (int add_z = -1; add_z <= 1; add_z+=2) {
                if (frustum.check_point(glm::vec3(node.center.x + add_x * node.side_len/2,
                                                  node.center.y + add_y * node.side_len/2,
                                                  node.center.z + add_z * node.side_len/2)))
                    count++;
                else if (count != 0)
                    return PARTIALLY_INSIDE;
            }
        }
    }
    return count ? FULLY_INSIDE : OUTSIDE;
}

void traverse_tree(frustum frustum, octree_node root, octree<int>& bbox_octree, std::vector<bool>& obj_included, int max_cull_depth)
{
    if (root.children_are_leaves) {
//        for (int val : bbox_octree.get_leaf_values(root))
//            obj_included[val] = true;
        for (auto vec : bbox_octree.get_leaf_values(root))
            for (int val : *vec)
                obj_included[val] = true;
        return;
    }
    if (max_cull_depth == 0) {
        for (int val : bbox_octree.get_all_leaves(root))
            obj_included[val] = true;
        return;
    }

    for (octree_node node : bbox_octree.get_children_of_node(root)) {
        int inside_count = frustum_intersects_octree_subdiv_count(node, frustum);
        if (inside_count) {
            if (inside_count == FULLY_INSIDE)
                for (int val : bbox_octree.get_all_leaves(node))
                    obj_included[val] = true;
            else if (inside_count == PARTIALLY_INSIDE)
                traverse_tree(frustum, node, bbox_octree, obj_included, max_cull_depth - 1);
        }
    }
}

std::vector<int> get_object_indices(frustum frustum, octree<int>& bbox_octree, int obj_count, int max_cull_depth=-1) 
{
    std::vector<bool> obj_included(obj_count, false);

    traverse_tree(frustum, bbox_octree.get_root(), bbox_octree, obj_included, ((max_cull_depth==-1) ? bbox_octree.max_depth : max_cull_depth));

    std::cout << check_count << " frustum-point intersection tests" << '\n';

    std::vector<int> out;
    out.reserve(obj_count);
    for (int i=0; i<obj_count; i++) {
        if (obj_included[i])
            out.push_back(i);
    }
    return out;
}


#include "ll_octree.hpp"
#include "frustum.hpp"
#include <iostream>

#include <vector>

bool frustum_intersects_octree_subdiv(octree_node node, frustum frustum)
{
    for (int add_x = -1; add_x <= 1; add_x+=2) {
        for (int add_y = -1; add_y <= 1; add_y+=2) {
            for (int add_z = -1; add_z <= 1; add_z+=2) {
                if (frustum.check_point(glm::vec3(node.center.x + add_x * node.side_len/2,
                                                  node.center.y + add_y * node.side_len/2,
                                                  node.center.z + add_z * node.side_len/2)))
                    return true;
            }
        }
    }
    return false;
}

void traverse_tree(frustum frustum, octree_node root, octree<int>& bbox_octree, std::vector<bool>& obj_included)
{
    if (root.children_are_leaves) {
        for (int val : bbox_octree.get_leaf_values(root))
            obj_included[val] = true;
/*        for (auto vec : bbox_octree.get_leaf_values(root))
            for (int val : *vec)
                obj_included[val] = true;*/
        return;
    }

    for (octree_node node : bbox_octree.get_children_of_node(root)) {
        if (frustum_intersects_octree_subdiv(node, frustum)) {
            traverse_tree(frustum, node, bbox_octree, obj_included);
        }
    }
}

std::vector<int> get_object_indices(frustum frustum, octree<int>& bbox_octree, int obj_count) 
{
    std::vector<bool> obj_included(obj_count, false);


    traverse_tree(frustum, bbox_octree.get_root(), bbox_octree, obj_included);

    std::vector<int> out;
    out.reserve(obj_count);
    for (int i=0; i<obj_count; i++) {
        if (obj_included[i])
            out.push_back(i);
    }
    return out;
}


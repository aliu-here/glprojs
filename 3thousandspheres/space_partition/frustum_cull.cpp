#include "octree.hpp"
#include "frustum.hpp"

#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

#ifdef DEBUG
int check_count = 0;
#endif

enum frustum_result {
    FULLY_INSIDE,
    PARTIALLY_INSIDE,
    OUTSIDE
};

frustum_result frustum_intersects_octree_subdiv_count(octree_node node, frustum frustum)
{
    int count = 0;
    for (int add_x = -1; add_x <= 1; add_x+=2) {
        for (int add_y = -1; add_y <= 1; add_y+=2) {
            for (int add_z = -1; add_z <= 1; add_z+=2) {
#ifdef DEBUG
                check_count++;
#endif
                if (frustum.check_point(glm::vec3(node.center.x + add_x * node.side_len/2,
                                                  node.center.y + add_y * node.side_len/2,
                                                  node.center.z + add_z * node.side_len/2))) {
                    count++;
                } else if (count != 0) {
                    return PARTIALLY_INSIDE;
                }
            }
        }
    }
    return (count == 0) ? OUTSIDE : FULLY_INSIDE;
}

void traverse_tree(frustum frustum, octree_node root, octree<int>& bbox_octree, std::vector<bool>& sure_included, std::vector<bool>& unsure_included, int max_cull_depth)
{
    if (root.children_are_leaves) {
//        for (int val : bbox_octree.get_leaf_values(root))
//            obj_included[val] = true;
        for (auto vec : bbox_octree.get_leaf_values(root))
            for (int val : *vec)
                unsure_included[val] = true;
        return;
    }
    if (max_cull_depth == 0) {
        for (int val : bbox_octree.get_all_leaves(root))
            unsure_included[val] = true;
        return;
    }

    for (octree_node node : bbox_octree.get_children_of_node(root)) {
        frustum_result inside_count = frustum_intersects_octree_subdiv_count(node, frustum);
        if (inside_count == FULLY_INSIDE)
            for (int val : bbox_octree.get_all_leaves(node))
                sure_included[val] = true;
        else if (inside_count == PARTIALLY_INSIDE)
            traverse_tree(frustum, node, bbox_octree, sure_included, unsure_included, max_cull_depth - 1);
    }
}

std::tuple<std::vector<int>, std::vector<int>> get_object_indices(frustum frustum, octree<int>& bbox_octree, int obj_count, int max_cull_depth=-1) 
{
    std::vector<bool> sure_included(obj_count, false), unsure_included(obj_count, false);

    traverse_tree(frustum, bbox_octree.get_root(), bbox_octree, sure_included, unsure_included, ((max_cull_depth==-1) ? bbox_octree.max_depth : max_cull_depth));

#ifdef DEBUG
    std::cout << check_count << " frustum-point intersection tests" << '\n';
#endif

    std::vector<int> sure_inside, unsure_inside;
    sure_inside.reserve(obj_count);
    unsure_inside.reserve(obj_count);
    for (int i=0; i<obj_count; i++) {
        if (sure_included[i])
            sure_inside.push_back(i);
        if (unsure_included[i])
            unsure_inside.push_back(i);
    }
    return {sure_inside, unsure_inside};
}


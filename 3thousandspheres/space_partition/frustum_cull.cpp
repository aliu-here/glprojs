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

enum further_checking_status {
    SURE_OUTSIDE,
    SURE_INSIDE,
    UNSURE
};

frustum_result frustum_intersects_octree_subdiv_count(octree_node node, frustum frustum)
{
    float s = node.side_len / 2;
    frustum::inside_degrees result = frustum.check_sphere(node.center, s * s * s / 8);
    if (result == frustum.FULLY_INSIDE)
        return FULLY_INSIDE;
    else if (result == frustum.PARTIALLY_INSIDE)
        return PARTIALLY_INSIDE;
    return OUTSIDE;
/*    int count = 0;
    
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
    return (count == 0) ? OUTSIDE : FULLY_INSIDE; */
}

void traverse_tree(frustum frustum, octree_node root, octree<int>& bbox_octree, std::vector<bool>& poss_included, int max_cull_depth)
{
    if (root.children_are_leaves) {
//        for (int val : bbox_octree.get_leaf_values(root))
//            obj_included[val] = true;
        for (auto vec : bbox_octree.get_leaf_values(root))
            for (int val : *vec)
                poss_included[val] = UNSURE;
        return;
    }
    if (max_cull_depth == 0) {
        for (int val : bbox_octree.get_all_leaves(root))
            poss_included[val] = UNSURE;
        return;
    }

    for (octree_node node : bbox_octree.get_children_of_node(root)) {
        frustum_result inside_count = frustum_intersects_octree_subdiv_count(node, frustum);
        if (inside_count == FULLY_INSIDE)
            for (int val : bbox_octree.get_all_leaves(node))
                poss_included[val] = SURE_INSIDE;
        else if (inside_count == PARTIALLY_INSIDE) {
            traverse_tree(frustum, node, bbox_octree, poss_included, max_cull_depth - 1);
        }
    }
}

std::tuple<std::vector<int>, std::vector<int>> get_object_indices(frustum frustum, octree<int>& bbox_octree, int obj_count, int max_cull_depth=-1) 
{
    std::vector<bool> poss_included(obj_count, SURE_OUTSIDE);

    traverse_tree(frustum, bbox_octree.get_root(), bbox_octree, poss_included, ((max_cull_depth==-1) ? bbox_octree.max_depth : max_cull_depth));

#ifdef DEBUG
    std::cout << check_count << " frustum-point intersection tests" << '\n';
#endif

    std::vector<int> sure_inside, unsure_inside;
    sure_inside.reserve(obj_count);
    unsure_inside.reserve(obj_count);
    for (int i=0; i<obj_count; i++) {
        switch (poss_included[i]) {
            case SURE_INSIDE:
                unsure_inside.push_back(i);
                break;
            case UNSURE:
                sure_inside.push_back(i);
                break;
        }
    }
    return {sure_inside, unsure_inside};
}


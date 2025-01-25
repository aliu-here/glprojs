#include <vector>
#include <array>
#include <glm/glm.hpp>

#ifndef OCTREE_HPP
#define OCTREE_HPP

class octree_node 
{
    public:
    octree_node(glm::vec3 center, float sidelen, int depth): center{center}, side_len{sidelen}, depth{depth}
    {
        if (depth == 0)
            children_are_leaves = true;
        else
            children_are_leaves = false;
    }

    int depth;
    glm::vec3 center;
    float side_len;
    bool children_are_leaves = false;
    std::array<std::array<std::array<int, 2>, 2>, 2> children = {{
                                                                  {{
                                                                    {{-1, -1}},
                                                                    {{-1, -1}}
                                                                  }},
                                                                  {{
                                                                    {{-1, -1}},
                                                                    {{-1, -1}}
                                                                  }}
                                                                }};
};

template <typename T> class octree 
{
    public:
    octree(glm::vec3 orig_center={0, 0, 0}, float orig_sidelen=100, int max_depth = 5): max_depth{max_depth}
    {
        octree_node root = octree_node(orig_center, orig_sidelen, max_depth);
        nodes.push_back(root);
    }

    void add_point(glm::vec3 point, T assoc_val) 
    {
        int curr_node = 0;

        while (!nodes[curr_node].children_are_leaves) {
            bool x_greater = point.x > nodes[curr_node].center.x;
            bool y_greater = point.y > nodes[curr_node].center.y;
            bool z_greater = point.z > nodes[curr_node].center.z;

            if (nodes[curr_node].children[x_greater][y_greater][z_greater] == -1) {
                float new_side = nodes[curr_node].side_len / 4;
                glm::vec3 new_center = {nodes[curr_node].center.x + (x_greater ? new_side : -new_side),
                                        nodes[curr_node].center.y + (y_greater ? new_side : -new_side),
                                        nodes[curr_node].center.z + (z_greater ? new_side : -new_side)};
                nodes.push_back(octree_node(new_center, new_side * 2, nodes[curr_node].depth - 1));
                nodes[curr_node].children[x_greater][y_greater][z_greater] = nodes.size() - 1;
            }
            curr_node = nodes[curr_node].children[x_greater][y_greater][z_greater];
        }

        bool x_greater = point.x > nodes[curr_node].center.x;
        bool y_greater = point.y > nodes[curr_node].center.y;
        bool z_greater = point.z > nodes[curr_node].center.z;


        if (nodes[curr_node].children[x_greater][y_greater][z_greater] == -1) {
            leaves.push_back(std::vector<T>());
            nodes[curr_node].children[x_greater][y_greater][z_greater] = leaves.size() - 1;
        }

        leaves[nodes[curr_node].children[x_greater][y_greater][z_greater]].push_back(assoc_val);
    }

    void remove_point(glm::vec3 point, T val)
    {
        octree_node& curr_node = nodes[0];

        while (!curr_node.children_are_leaves) {
            bool x_greater = point.x > curr_node.center.x;
            bool y_greater = point.y > curr_node.center.y;
            bool z_greater = point.z > curr_node.center.z;

            if (curr_node.children[x_greater][y_greater][z_greater] == -1) {
                return;
            }
            curr_node = nodes[curr_node.children[x_greater][y_greater][z_greater]];
        }

        bool x_greater = point.x > curr_node.center.x;
        bool y_greater = point.y > curr_node.center.y;
        bool z_greater = point.z > curr_node.center.z;


        if (curr_node.children[x_greater][y_greater][z_greater] == -1) {
            return;
        }

        leaves[curr_node.children[x_greater][y_greater][z_greater]].erase(val);
    }

    std::vector<octree_node> get_children_of_node(octree_node node)
    {
        if (node.children_are_leaves)
            return {};
        std::vector<octree_node> out;
        for (auto i : node.children) {
            for (auto j : i) {
                for (auto k : j) {
                    if (k == -1) {
                        continue;
                    }
                    out.push_back(nodes[k]);
                }
            }
        }
        return out;
    }

    const octree_node& get_node_at_idx(int idx)
    {
        return nodes[idx];
    }

    std::vector<T> get_leaf_values(octree_node node)
    {
        std::vector<T> out;

        for (auto half1 : node.children) {
            for (auto half2 : half1) {
                for (auto half3 : half2) {
                    if (half3 == -1) {
                        continue;
                    }
                    if (node.children_are_leaves) {
                        out.insert(out.end(), leaves[half3].begin(), leaves[half3].end());
                    } else {
                        std::vector<T> temp = get_leaf_values(nodes[half3]);
                        out.insert(out.end(), temp.begin(), temp.end());
                    }
                }
            }
        }
        return out;
    }

    private:

    int max_depth;
    std::vector<octree_node> nodes;
    std::vector<std::vector<T>> leaves;
};

#endif

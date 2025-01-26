#include "ll_octree.hpp"
#include "frustum.hpp"

#include <vector>

#ifndef FRUSTUM_CULL
#define FRUSTUM_CULL
std::vector<int> get_object_indices(frustum frustum, octree<int>& bbox_octree, int obj_count);
#endif

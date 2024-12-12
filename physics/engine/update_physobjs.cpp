#include "physobj.hpp"
#include <vector>

void update_objs(std::vector<physics_obj*> &objs, float deltatime) 
{
    /*
     * deltatime should be in seconds; return from glfwGetTime();
     */
    const glm::vec3 gravity = {0, -9.8, 0};
    //velocity verlet integration
    for (int i=0; i<objs.size(); i++) {
        objs[i]->pos += objs[i]->velocity * deltatime + objs[i]->acceleration * (deltatime * deltatime * 0.5f);
        glm::vec3 newacceleration = gravity;
        objs[i]->velocity += (objs[i]->acceleration + newacceleration) * 0.5f * deltatime;
        objs[i]->acceleration = newacceleration;
    }
};

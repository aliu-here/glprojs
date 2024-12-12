#include "../wavefront_loader/mesh.hpp"
#include <vector>

struct physics_obj
{
    loader::model model;
    glm::vec3 pos;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    physics_obj(loader::model model) 
    {
        this->model = model;
        pos = {0, 0, 0};
        velocity = {0, 0, 0};
        acceleration = {0, 0, 0};
    }
};

void update_objs(std::vector<physics_obj*>& objs, float deltatime);

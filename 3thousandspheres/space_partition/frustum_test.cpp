#include "frustum.hpp"
#include <glm/gtc/matrix_transform.hpp>

int main()
{
    glm::mat4 mat = glm::mat4(1.0f);

    mat = glm::perspective(glm::radians(45.0f), 1.f, 1.f, 1000.0f);

    frustum f = frustum(mat);

    f.print_mat(mat);

    f.print_vec3(f.bottom);
    f.print_vec3(f.top);

    f.print_vec3(f.left);
    f.print_vec3(f.right);

    f.print_vec3(f.near);
    f.print_vec3(f.far);
}


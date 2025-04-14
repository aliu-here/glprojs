#include <glm/glm.hpp>

#ifndef FRUSTUM
#define FRUSTUM

struct frustum
{

/*    void print_mat(glm::mat4 mat)
    {
        for (int i=0; i<4; i++) {
            for (int j=0; j<4; j++) {
                std::cout << i << j << ",: " << mat[i][j] << '\t';
            }
            std::cout << '\n';
        }
    }

    void print_vec3(glm::vec4 vec) {
        std::cout << "x: " << vec.x << ", y: " << vec.y << ", z: " << vec.z << ", w: "  << vec.w<<"\n";
    }*/

    glm::vec4 normalize_vec4(glm::vec4 vec)
    {
        float length = glm::length(glm::vec3(vec.x, vec.y, vec.z));
        return vec / length;
    }

    frustum(glm::mat4 mat)
    { //gribb-hartmann
      mat = glm::transpose(mat);
      left = normalize_vec4(mat[3] + mat[0]);
      right = normalize_vec4(mat[3] - mat[0]);
      bottom = normalize_vec4(mat[3] + mat[1]);
      top = normalize_vec4(mat[3] - mat[1]);
      near = normalize_vec4(mat[3] + mat[2]);
      far = normalize_vec4(mat[3] - mat[2]);
    }

    float plane_eq(glm::vec4 plane_vec, glm::vec3 point)
    {
        return -glm::dot(glm::vec3(plane_vec.x, plane_vec.y, plane_vec.z), point) - plane_vec.w;
    }

    bool check_point(glm::vec3 point) 
    {
        return (plane_eq(left, point) < 0 && plane_eq(right, point) < 0 &&\
                plane_eq(bottom, point) < 0 && plane_eq(top, point) < 0 && \
                plane_eq(near, point) < 0 && plane_eq(far, point) < 0);
    }

    bool check_sphere(glm::vec3 point, float radius)
    {
        return (!(plane_eq(left, point) < -radius) && !(plane_eq(right, point) < -radius) &&\
                !(plane_eq(bottom, point) < -radius) && !(plane_eq(top, point) < -radius) && \
                !(plane_eq(near, point) < -radius) && !(plane_eq(far, point) < -radius));

    }

    glm::vec4 left, right, bottom, top, near, far;
};

#endif

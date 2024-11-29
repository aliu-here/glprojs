#include <array>
#include <cmath>
#include <glm/ext/vector_float2.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include "mesh.hpp"
#include "string_utils.hpp"

enum windings {
    CW=false,
    CCW=true
};

enum project_directions {
    x=0,
    y=1,
    z=2 
};

namespace loader 
{
    //debugging functions
    void print_vec3(glm::vec3 in) {
        std::cout << "x: " << in.x << " y: " << in.y << " z: " << in.z << '\n';
    }

    void print_point(point in) {
        std::cout << "x: " << in.coord.x << " y: " << in.coord.y << " z: " << in.coord.z << \
            " uv_x: " << in.tex.x << " uv_y: " << in.tex.y << \
            " normal_x: " << in.normal.x << " normal_y: " << in.normal.y << " normal_z: " << in.normal.z << '\n';
    }

    //this is probably a bad idea; this turns the point struct into a string so it can be used in an unordered_map
    std::string point_to_string(point point)
    {
        return std::string(reinterpret_cast<char*>(&point), sizeof(point));
    }
    
    std::string vec3_to_string(glm::vec3 vec)
    {
        return std::string(reinterpret_cast<char*>(&vec), sizeof(vec));
    }

    std::array<unsigned int, 3> wind_ccw_coords_triangle(std::array<glm::vec3, 3> coords, std::array<unsigned int, 3> vertices, glm::vec3 normal, bool input_winding=CW)
    {
        glm::vec3 side_a = coords[1] - coords[0];
        glm::vec3 side_b = coords[2] - coords[0];
        //use ccw winding
        glm::vec3 cross = glm::cross(side_a, side_b);
        if (glm::dot(cross, normal) < 0)
            //weird manual cast
            return input_winding ? (std::array<unsigned int, 3>){vertices[0], vertices[1], vertices[2]} : (std::array<unsigned int, 3>){vertices[0], vertices[2], vertices[1]};
        return input_winding ? (std::array<unsigned int, 3>){vertices[0], vertices[2], vertices[1]} : (std::array<unsigned int, 3>){vertices[0], vertices[1], vertices[2]};
    }

    glm::vec3 project_in_dir(glm::vec3 vec, int project_dir) 
    {
//        std::cout << project_dir << '\n';
        switch (project_dir) {
            case x:
                return glm::vec3(0, vec[1], vec[2]);
            case y:
                return glm::vec3(vec[0], 0, vec[2]);
            case z:
                return glm::vec3(vec[0], vec[1], 0);
            default:
                std::cout << "error, project_dir handed to project_in_direction is not 0, 1, or 2 (x, y, or z)\n";
                return glm::vec3(0, 0, 0);
        }
    }

    int find_project_dir(glm::vec3 cross_product) 
    {
        glm::vec3 cross_prod_norm = glm::normalize(cross_product);
        float angle_to_x_axis = cross_prod_norm[0],
              angle_to_y_axis = cross_prod_norm[1],
              angle_to_z_axis = cross_prod_norm[2];
        float best_angle = 0; // we want to find angle where |angle| is closest to one
                              // because |cos(0)| and |cos(180)| = 1
        int project_dir = 0;
        if (std::abs(angle_to_x_axis) > project_dir) {
            project_dir = x;
        } else if (std::abs(angle_to_y_axis) > project_dir) {
            project_dir = y;
        } else if (std::abs(angle_to_z_axis) > project_dir) {
            project_dir = z;
        }
        return project_dir;
    }


    bool angle_direction_is_into_polygon(std::vector<point>& points, int center_index) 
    {
        glm::vec3 side_a = points[(center_index + 1) % points.size()].coord - points[center_index].coord;
        glm::vec3 side_b = points[(center_index - 1 + points.size()) % points.size()].coord - points[center_index].coord;
        int project_dir = find_project_dir(glm::cross(side_a, side_b));
        glm::vec3 ray_start = project_in_dir(points[center_index].coord, project_dir),
                  ray_dir = project_in_dir(side_a + side_b, project_dir);
        
        int cross_count = 0; 

        const float EPSILON = std::pow(2, -10);

        for (int seg_start_pos=0; seg_start_pos < points.size(); seg_start_pos++) {
            glm::vec3 seg_start = project_in_dir(points[seg_start_pos].coord, project_dir),
                      seg_dir = project_in_dir(points[(seg_start_pos + 1) % points.size()].coord, project_dir) - seg_start;

            //algorithm from page 304 of graphics gems 1; by ronald goldman
            glm::vec3 v1_cross_v2 = glm::cross(ray_dir, seg_dir);
            float v1_cross_v2_squared = glm::dot(v1_cross_v2, v1_cross_v2);

            if (v1_cross_v2_squared == 0.0f)
                continue; //this will happen if parallel

            float t_numerator = glm::dot(glm::cross((seg_start - ray_start), seg_dir), v1_cross_v2),
                  t_denominator = v1_cross_v2_squared,
                  t = t_numerator / t_denominator;
            float s_numerator = glm::dot(glm::cross((seg_start - ray_start), ray_dir), v1_cross_v2),
                  s_denominator = v1_cross_v2_squared,
                  s = s_numerator / s_denominator;

            //check if it's actually intersecting within the bounds
//            std::cout << "ray intersect point: ";
//            print_vec3((ray_start + ray_dir*t));
//            std::cout << "segment intersect point: ";
//            print_vec3((seg_start + seg_dir*s));
//            std::cout << "t: " << t << " s: " << s << "\n";
            if (t <= 0) //don't count starting vertex
                continue;
            if (s < 0 || s >= 1) // if s > 1 then it will be outside the bounds of the side
                continue;        // and if s = 1 then you'll be double counting corners if they intersect
            if (glm::length((ray_start + ray_dir*t) - (seg_start + seg_dir*s)) <= EPSILON) {
//                std::cout << "same" << '\n';
                cross_count++;
            }
        }
        return (cross_count % 2);

    }


    //triangulate polygon using ear clipping, for polygons with >4 vertices (bad, should probably redo)
    std::vector<std::array<unsigned int, 3>> triangulate_poly(std::vector<point>& points, std::unordered_map<std::string, unsigned int>& indices, bool winding=CW)
    {
/*        for (auto point : points) {
            print_vec3(point.coord);
        }*/
        std::vector<std::array<unsigned int, 3>> out;
        int prevsize = points.size();
        int currsize = points.size();
        while (points.size() > 3) {
            for (int point_index = 0; point_index < points.size(); point_index++) {
                int prev_index = (point_index - 1 + points.size()) % points.size(), next_index = (point_index + 1) % points.size();
                if (angle_direction_is_into_polygon(points, point_index)) {
                    std::array<unsigned int, 3> wound_triangle = wind_ccw_coords_triangle({points[prev_index].coord, 
                                                                                  points[point_index].coord,
                                                                                  points[next_index].coord},
                                                                                 {indices[point_to_string(points[prev_index])],
                                                                                  indices[point_to_string(points[point_index])],
                                                                                  indices[point_to_string(points[next_index])]},
                                                                                 points[point_index].normal,
                                                                                 winding);
                    out.push_back(wound_triangle);
                    points.erase(points.begin() + point_index);
                    break;
                }
            }
            currsize = points.size();
            if (currsize == prevsize) {
                std::cout << "error\n";
                std::cout << "points: \n";
                for (auto point : points) {
                    print_point(point);
                }
                exit(1);
            }
        }
        std::array<unsigned int, 3> wound_triangle = wind_ccw_coords_triangle({points[0].coord, 
                                                                               points[1].coord,
                                                                               points[2].coord},
                                                                              {indices[point_to_string(points[0])],
                                                                               indices[point_to_string(points[1])],
                                                                               indices[point_to_string(points[2])]},
                                                                              points[1].normal,
                                                                              winding);
        out.push_back(wound_triangle);
        return out;
    }

    point generate_point(const std::string& point_data, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals)
    {
//        std::cout << "point_data=" << point_data << '\n';
        std::vector<std::string> split_arg = split(point_data, "/");
        bool error_detected = false, include_tex_data = false, include_normal_data = false;
        point curr_point;
        std::vector<int> locs;
        for (std::string num: split_arg)
        {
            if (num == std::string(""))
                locs.push_back(0);
            else
                try {
                    locs.push_back(std::stoi(num));
                } catch (std::invalid_argument) {
                    error_detected = true;
                }
        }
        if (locs.size() < 1 || locs.size() > 3 || error_detected == true)
            std::cerr << "Error in .obj file detected\n";
        if (locs[0] < 0)
            locs[0] += coords.size(); //.obj allows negative numbers; from back
        else
            locs[0] -= 1;
        curr_point.coord = coords[locs[0]];
        switch(locs.size())
        {
            case 1:
                include_tex_data = false;
                include_normal_data = false;
                break;
            case 2:
                include_tex_data = true;
                include_normal_data = false;
                break;
            case 3:
                if (locs[1] == 0)
                    include_tex_data = false;
                else 
                    include_tex_data = true;
                include_normal_data = true;
                break;
        }
        if (include_tex_data) {
            if (locs[1] < 0)
                locs[1] += tex.size(); //.obj allows negative numbers; from back
            else
                locs[1] -= 1;
            curr_point.tex = tex[locs[1]];
        }
        if (include_normal_data) {
            if (locs[2] < 0)
                locs[2] += normals.size(); //.obj allows negative numbers; from back
            else
                locs[2] -= 1;
            curr_point.normal = normals[locs[2]];
        }
        return curr_point;
    }

    std::tuple<std::vector<point>*, std::vector<std::array<unsigned int, 3>>* >* process_faces(const std::vector<std::string>& faces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals)
    { //this is extremely stupid
        std::vector<std::array<unsigned int, 3>> *point_indexes = new std::vector<std::array<unsigned int, 3>>;
        std::vector<point> *all_points = new std::vector<point>;
#ifdef DEBUG
        std::cout << "face count: " << faces.size() << '\n';
#endif
        int points_sofar=0;
        for (std::string line : faces)
        {
            std::vector<point> point_listing;
            std::unordered_map<std::string, unsigned int> indices;
            int point_count = 0;
            std::vector<std::string> split_face = split(line, " ");
//            std::cout << line << '\n';
            for (int i=1; i<split_face.size(); i++)
            {
                point curr_point = generate_point(split_face[i], coords, tex, normals);
                std::string serialized = point_to_string(curr_point);
                if (indices.find(serialized) == indices.end())
                {
                    indices.insert({serialized, point_count + points_sofar});
                    point_listing.push_back(curr_point);
                    point_count++;
                }
            }

            points_sofar += point_listing.size();

            for (auto point : point_listing) {
//                print_point(point);
            }

            all_points->insert(all_points->end(), point_listing.begin(), point_listing.end());
            std::vector<std::array<unsigned int, 3>> temp = triangulate_poly(point_listing, indices);
            if (temp.size() == 0)
                return {};

            point_indexes->insert(point_indexes->end(), temp.begin(), temp.end());
//            std::cout << points_sofar << ": points_sofar\n";
//            std::cout << std::endl << '\n';
        }

#ifdef DEBUG
        for (point x : all_points)
            print_point(x);
        for (auto x : point_indexes) {
            for (int i=0; i<3; i++) {
                std::cout << x[i] << ' ';
            }
            std::cout << '\n';
        }
#endif
        std::tuple<std::vector<point>*, std::vector<std::array<unsigned int, 3>>* > *temp = new std::tuple<std::vector<point>*, std::vector<std::array<unsigned int, 3>>* >;
        std::get<0>(*temp) = all_points;
        std::get<1>(*temp) = point_indexes;
        return temp;
    }
}

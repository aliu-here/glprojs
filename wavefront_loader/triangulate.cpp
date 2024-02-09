#include <array>
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

    //this is probably a bad idea
    std::string serialize_point(point point)
    {
        return std::string(reinterpret_cast<char*>(&point), sizeof(point));
    }

    std::array<int, 3> wind_ccw_coords_triangle(std::array<glm::vec3, 3> coords, std::array<int, 3> vertices, glm::vec3 normal, bool input_winding=CW) //you could find the winding but it's probably easier to pass it in
    {
        glm::vec3 side_a = coords[1] - coords[0];
        glm::vec3 side_b = coords[2] - coords[0];
        //use ccw winding
        glm::vec3 cross = glm::cross(side_a, side_b);
        if (glm::dot(cross, normal) < 0)
            //weird manual cast
            return input_winding ? (std::array<int, 3>){vertices[0], vertices[2], vertices[1]} : (std::array<int, 3>){vertices[0], vertices[1], vertices[2]};
        return input_winding ? (std::array<int, 3>){vertices[0], vertices[1], vertices[2]} : (std::array<int, 3>){vertices[0], vertices[2], vertices[1]} ;
    }

    //triangulate polygon using ear clipping
    std::vector<std::array<int, 3>> triangulate_poly(std::vector<point>& points, std::unordered_map<std::string, int>& indices, const std::string& line, bool winding=CW)
    {
        std::vector<std::array<int, 3>> output;
        int prev_size = 0, curr_size = points.size();
        const int points_size = points.size();
        int next, prev;
        std::vector<glm::vec3> crosses = {}; // cross product of first and last, compare against rest
        int ignore_dir;
        float mindot = FLT_INF;
        bool first = true;
        for (int i=0; i<points_size; i++) {
            next = (i+1) % points_size;
            prev = (i + points_size - 1) % points_size;
            glm::vec3 side1, side2;
            side1 = points[next].coord - points[i].coord;
            side2 = points[i].coord - points[prev].coord;
            glm::vec3 cross = glm::cross(side2, side1); // keep consistent with earlier, later then first bc non-commutative
            std::cout << "current cross: ";
            print_vec3(cross);
            if (first) {
                crosses.push_back(cross);
                first = false;
                continue;
            }
            std::cout << "first cross: ";
            print_vec3(crosses[0]);
            std::cout << glm::dot(glm::normalize(cross), glm::normalize(crosses[0])) << '\n';

            if (fabs(glm::dot(glm::normalize(cross), glm::normalize(crosses[0]))) != 1) { // make sure all of the cross products are pointing in the same direction (or the exact opposite)
                std::cout << "the polygon is not flat, or there are three consecutive collinear points\n";
                return {};
            }
            crosses.push_back(cross);
        }
        float xdotprod, ydotprod, zdotprod;
        if (crosses[0].x != 0) 
            ignore_dir = x;
        else if (crosses[0].y != 0) 
            ignore_dir = y;
        else if (crosses[0].z != 0) 
            ignore_dir = z;
        std::cout << "ignore_dir=" << ignore_dir << '\n';
        for (int i=0; i<crosses.size(); i++)
            std::cout << "cross product: " << crosses[i][ignore_dir] << ", index: " << i << '\n';
//        while (points.size() > 3){
            /*
            if (prev_size == curr_size) {
                std::cout << line << '\n';
                for (point curr_point: points){
                    print_point(curr_point);
                }
                assert(prev_size != curr_size);
            }
            glm::vec3 firstcross = glm::normalize(glm::cross((points[1].coord - points[0].coord),(points[points.size() - 1].coord - points[0].coord)));
            for (int curr=0; curr<points.size(); curr++)
            {
                std::cout << '\n';
                std::array<int, 3> curr_coord;
                int prev, next;
                next = (curr + 1) % points.size();
                prev = curr - 1;
                if (prev == -1)
                    prev = points.size() - 1;

                glm::vec3 line2 = points[next].coord - points[curr].coord; //check if collinear
                glm::vec3 line1 = points[prev].coord - points[curr].coord;
                glm::vec3 cross = glm::normalize(glm::cross(line1, line2));
                glm::vec3 sideline = points[(next + 1) % points.size()].coord - points[next].coord;
                glm::vec3 line3 = points[next].coord - points[prev].coord;
                if ((cross != firstcross && cross != -firstcross)) {
                    std::cerr << "Points on face \"" << line << "\" are not all coplanar\n";
                    return {};
                }
                if (cross == glm::vec3(0, 0, 0))
                    continue;

                for (point x: points)
                    print_point(x);

                //cursed linear algebra coming ahead
                float line2_to_sideline_angle = acos(glm::dot(line2, sideline) / (glm::length(line2) * glm::length(sideline)));
                float line3_to_sideline_angle = acos(glm::dot(line3, sideline) / (glm::length(line3) * glm::length(sideline)));
                float line3_to_line2_angle = acos(glm::dot(line3, line2) / (glm::length(line3) * glm::length(line2)));
                if (line3_to_sideline_angle - line3_to_line2_angle == line2_to_sideline_angle) //this breaks in some cases but idrc
                    continue;

                curr_coord = {indices[serialize_point(points[curr])],
                              indices[serialize_point(points[prev])],
                              indices[serialize_point(points[next])]};
                curr_coord = wind_ccw_coords_triangle({points[curr].coord, points[prev].coord, points[next].coord},
                                                     curr_coord,
                                                     points[curr].normal);
                output.push_back(curr_coord);
                points.erase(points.begin() + curr);
                break;
            }
            prev_size = curr_size;
            curr_size = points.size();
            */
//        }
//        output.push_back(wind_ccw_coords_triangle({points[0].coord, points[1].coord, points[2].coord},
//                                                 {indices[serialize_point(points[0])],
//                                                  indices[serialize_point(points[1])],
//                                                  indices[serialize_point(points[2])]},
//                                                 points[0].normal));
        return output;
    }

    point generate_point(const std::string& point_data, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2> tex, const std::vector<glm::vec3> normals, int line_num, int coord_offset, int tex_offset, int normal_offset)
    {
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
            std::cerr << "Error in .obj file at line " << line_num << '\n';
        if (locs[0] < 0)
            locs[0] += coords.size(); //.obj allows negative numbers; from back
        else 
            locs[0] -= 1 + coord_offset; //.obj is 1-indexed
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
                locs[1] -= 1 + tex_offset; //.obj is 1-indexed
            curr_point.tex = tex[locs[1]];
        }
        if (include_normal_data) {
            if (locs[2] < 0)
                locs[2] += normals.size(); //.obj allows negative numbers; from back
            else 
                locs[2] -= 1 + normal_offset; //.obj is 1-indexed
            curr_point.normal = normals[locs[2]];
        }
        return curr_point;
    }

    std::tuple<std::vector<point>, std::vector<std::array<int, 3>>> process_faces(const std::vector<std::string>& faces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, const int line_num, const int coord_offset, const int tex_offset, const int normal_offset)
    {
        int offset = 0;
        std::vector<std::array<int, 3>> point_indexes;
        std::vector<point> all_points;
        for (std::string line : faces)
        {
            std::vector<point> point_listing;
            std::unordered_map<std::string, int> indices;
            int point_count = 0;
            std::vector<std::string> split_face = split(line, " ");
            for (int i=1; i<split_face.size(); i++)
            {
                point curr_point = generate_point(split_face[i], coords, tex, normals, line_num, coord_offset, tex_offset, normal_offset);
                std::string serialized = serialize_point(curr_point);
                if (indices.find(serialized) == indices.end())
                {
                    indices.insert({serialized, point_count});
                    point_listing.push_back(curr_point);
                    point_count++;
                }
            }
            all_points.insert(all_points.end(), point_listing.begin(), point_listing.end());
            std::vector<std::array<int, 3>> temp = triangulate_poly(point_listing, indices, line);
            if (temp.size() == 0)
                return {};
            point_indexes.insert(point_indexes.end(), temp.begin(), temp.end());
        }
        return {all_points, point_indexes};
    }
}

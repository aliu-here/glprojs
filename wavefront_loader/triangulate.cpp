#include <array>
#include <cmath>
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


        glm::vec3 side1, side2;
        side1 = points[1].coord - points[0].coord;
        side2 = points[0].coord - points[points_size-1].coord;
        std::vector<glm::vec3> crosses = {glm::cross(side2,
                                                     side1)}; // cross product of first and last, compare against rest
        int ignore_dir;
        float mindot = FLT_INF;
        glm::vec3 normalized_first = glm::normalize(crosses[0]), normalized_curr;
        std::vector<double> angles;

#ifdef DEBUG
        //calculate the first angle
        std::cout << "side1: ";
        print_vec3(side1);
        std::cout << "side2: ";
        print_vec3(side2);
#endif

        if (crosses[0].x != 0) 
            ignore_dir = x;
        else if (crosses[0].y != 0) 
            ignore_dir = y;
        else if (crosses[0].z != 0) 
            ignore_dir = z;
        std::cout << "ignore_dir: " << ignore_dir << '\n';
        
        for (int i=1; i<points_size; i++) {
            std::cout << '\n';
            next = (i+1) % points_size;
            prev = (i + points_size - 1) % points_size;
            side1 = points[next].coord - points[i].coord;
            side2 = points[i].coord - points[prev].coord;
            glm::vec3 cross = glm::cross(side2, side1); // keep consistent with earlier, later then first bc non-commutative
            normalized_curr = glm::normalize(cross);
#ifdef DEBUG
            std::cout << "current normalized cross: ";
            print_vec3(normalized_curr);
            std::cout << "first normalized cross: ";
            print_vec3(normalized_first);
            std::cout << "normalized side1: ";
            print_vec3(glm::normalize(side1));
            std::cout << "normalized side2: ";
            print_vec3(glm::normalize(side2));
#endif

            if ((normalized_curr != normalized_first) && ((-normalized_curr) != normalized_first)) { // make sure all of the cross products are pointing in the same direction (or the exact opposite)
                std::cerr << "the polygon is not flat, or there are three consecutive collinear points\n";
                return {};
            }

            crosses.push_back(cross);

            double angle = std::acos(glm::dot(side1, -side2) / (glm::length(side1) * glm::length(side2)));
#ifdef DEBUG
            std::cout << "angle: " << angle << ", index: " << i << '\n';
#endif
            if (std::signbit(crosses[i][ignore_dir]) && winding == CW || ~std::signbit(crosses[i][ignore_dir]) && winding == CCW)
                angle = (2*M_PI) - angle;
            angles.push_back(angle);
        }
        side1 = points[1].coord - points[0].coord;
        side2 = points[0].coord - points[points_size - 1].coord;
        double angle = std::acos(glm::dot(side1, -side2) / (glm::length(side1) * glm::length(side2)));
        if (std::signbit(crosses[0][ignore_dir]) && winding == CW || ~std::signbit(crosses[0][ignore_dir]) && winding == CCW)
            angle = (2*M_PI) - angle;
        angles.push_back(angle);
        double regularsum=0;
        
        for (int i=0; i<angles.size(); i++){
            regularsum += angles[i];
        }

        bool reverse = std::fabs(regularsum - (points_size - 2) * M_PI) > 0.00001f; //tolerate anything within that amount of error because float

        std::vector<float> crosses_from_2d;

#ifdef DEBUG
        std::cout << "ignore_dir=" << ignore_dir << '\n';

        for (int i=0; i<crosses.size(); i++)
            std::cout << "cross product: " << crosses[i][ignore_dir] << ", index: " << i << '\n';

        std::cout << "reverse: " << (reverse ? "yes" : "no") << '\n';

        for (point x : points)
            print_point(x);
#endif

//      actual triangulation code goes here
        int prevsize = 0, currsize = points_size;
        while (points.size() > 3) {
            if (prevsize == currsize) {
                std::cout << "points.size(): " << points.size() << '\n';
                std::cout << "winding: " << winding << '\n';
                assert(prevsize != currsize);
            }
            std::cout << '\n';
            for (int i=0; i<crosses.size(); i++) {
                std::cout << "adjusted cross: " << crosses[i][ignore_dir] * (reverse ? -1 : 1) << '\n';
                if (((crosses[i][ignore_dir] * (reverse ? -1 : 1)> 0) && winding == CW) || ((crosses[i][ignore_dir] * (reverse ? -1 : 1) < 0) && winding == CCW)) {
                    int prev, next;
                    prev = (points_size + i - 1) % points_size;
                    next = (i + 1) % points_size;
                    std::array<int, 3> temp = wind_ccw_coords_triangle( {points[prev].coord,
                                                                         points[i].coord,
                                                                         points[next].coord},
                                                                        {indices[serialize_point(points[prev])],
                                                                         indices[serialize_point(points[i])],
                                                                         indices[serialize_point(points[next])]},
                                                                        crosses[i],
                                                                        winding);
                    output.push_back(temp);
                    points.erase(points.begin() + i);
                    crosses.erase(crosses.begin() + i);
                    break;
                }
            }
            prevsize = currsize;
            currsize = points.size();
        }
        std::array<int, 3> temp = wind_ccw_coords_triangle( {points[2].coord,
                                                             points[0].coord,
                                                             points[1].coord},
                                                            {indices[serialize_point(points[2])],
                                                             indices[serialize_point(points[0])],
                                                             indices[serialize_point(points[1])]},
                                                            crosses[0],
                                                            winding);
        output.push_back(temp);
        return output;
    }

    point generate_point(const std::string& point_data, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, int line_num)
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

    std::tuple<std::vector<point>, std::vector<std::array<int, 3>>> process_faces(const std::vector<std::string>& faces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, int line_num)
    {
        std::vector<std::array<int, 3>> point_indexes;
        std::vector<point> all_points;
#ifdef DEBUG
        std::cout << "face count: " << faces.size() << '\n';
#endif
        int points_sofar=0;
        for (std::string line : faces)
        {
            std::vector<point> point_listing;
            std::unordered_map<std::string, int> indices;
            int point_count = 0;
            std::vector<std::string> split_face = split(line, " ");
            for (int i=1; i<split_face.size(); i++)
            {
                point curr_point = generate_point(split_face[i], coords, tex, normals, line_num);
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
            for (int i=0; i<temp.size(); i++) {
                for (short j=0; j<3; j++) {
                    temp[i][j] += points_sofar;
                }
            }
            points_sofar += point_listing.size();
            point_indexes.insert(point_indexes.end(), temp.begin(), temp.end());
            line_num++;
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
        return {all_points, point_indexes};
    }
}

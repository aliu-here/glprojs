#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <limits>
#include <chrono>
#include <cstring>
#include <array>
#include <tuple>
#include <thread>
#include <future>

#include "mesh.hpp"
#include "string_utils.hpp"

enum windings {
    CW=false,
    CCW=true
};

namespace loader
{
    //this is probably a bad idea
    std::string serialize_point(point point)
    {
        return std::string(reinterpret_cast<char*>(&point), sizeof(point));
    }

    std::array<int, 3> wind_coords_of_triangle(std::array<glm::vec3, 3> coords, std::array<int, 3> vertices, glm::vec3 normal, bool input_winding=CW)
    {
        glm::vec3 side_a = coords[1] - coords[0];
        glm::vec3 side_b = coords[2] - coords[0];
        //use ccw winding
        glm::vec3 cross = glm::cross(side_a, side_b);
        if (glm::dot(cross, normal) < 0)
            return input_winding ? (std::array<int, 3>){vertices[0], vertices[2], vertices[1]} : (std::array<int, 3>){vertices[0], vertices[1], vertices[2]};
        return input_winding ? (std::array<int, 3>){vertices[0], vertices[1], vertices[2]} : (std::array<int, 3>){vertices[0], vertices[2], vertices[1]} ;
    }
    //triangulate polygon using ear clipping
   
    void print_vec3(glm::vec3 in) {
        std::cout << "x: " << in.x << " y: " << in.y << " z: " << in.z << '\n';
    }
    std::vector<std::array<int, 3>> triangulate_poly(std::vector<point>& points, std::unordered_map<std::string, int>& indices, const std::string& line)
    {
        std::vector<std::array<int, 3>> output;
        while (points.size() > 3){
            for (int curr=0; curr<points.size(); curr++)
            {
                std::array<int, 3> curr_coord;
//                std::cout << '\n';
                int prev, next;
                next = (curr + 1) % points.size();
                prev = curr - 1;
                if (prev == -1)
                    prev = points.size() - 1;
//                std::cout << "prev=" << prev << " curr=" << curr << " next=" << next << '\n';
                glm::vec3 diffa = points[next].coord - points[curr].coord; //check if collinear
                glm::vec3 diffb = points[prev].coord - points[curr].coord;
//                for (auto it : points)
//                    print_vec3(it.coord);
//              std::cout << line << '\n';
                  if (glm::cross(diffa, diffb) == glm::vec3(0, 0, 0))
                    continue;
                curr_coord = {indices[serialize_point(points[curr])],
                              indices[serialize_point(points[prev])],
                              indices[serialize_point(points[next])]};
                curr_coord = wind_coords_of_triangle({points[curr].coord, points[prev].coord, points[next].coord},
                                                     curr_coord,
                                                     points[curr].normal);
                output.push_back(curr_coord);
                points.erase(points.begin() + curr);
                break;
            }
        }
        output.push_back(wind_coords_of_triangle({points[0].coord, points[1].coord, points[2].coord},
                                                 {0, 1, 2},
                                                 points[0].normal));
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
            point_indexes.insert(point_indexes.end(), temp.begin(), temp.end());
        }
        return {all_points, point_indexes};
    }

    //this only works if the user gives the correct path 
    //loads a material, stores ambient, diffuse, specular as glm::vec3 and stores specular_exponent as float
    std::unordered_map<std::string, material> load_mtl(const std::string& path)
    {
        std::unordered_map<std::string, material> materials;
        std::vector<std::string> names;
        material curr_material;
        std::ifstream mtl_file;
        mtl_file.open(path);
        if (mtl_file.fail())
        {
            std::cerr << "Failed to open material file\n";
            return materials;
        }
        std::string curr_name;
        int line_num=0; //just to say where it went wrong
        for (std::string line; std::getline(mtl_file, line);)
        {
            bool check_1st=false, check_rest=false;
            line_num++;
            std::string str_arg1, str_arg2, str_arg3, line_type;
            float arg1, arg2, arg3;
            std::stringstream line_stream(line);
            line_stream >> line_type >> str_arg1 >> str_arg2 >> str_arg3;

            try {arg1 = std::stof(str_arg1);}
            catch (std::invalid_argument) {}

            try { arg1 = std::stof(str_arg1); } 
            catch (std::invalid_argument) {}
            try { arg2 = std::stof(str_arg2); } 
            catch (std::invalid_argument) {}
            try { arg3 = std::stof(str_arg3); } 
            catch (std::invalid_argument) {}

            try {
                if (line_type[0] == 'K') {
                    glm::vec3 curr_color = glm::vec3(arg1, arg2, arg3);
                    check_rest=true;
                    if (line_type == "Ka")
                        curr_material.ambient = curr_color;
                    else if (line_type == "Kd")
                        curr_material.diffuse = curr_color;
                    else if (line_type == "Ks")
                        curr_material.specular = curr_color;
                } else if (line_type == "Ns") {
                    curr_material.specular_exp = arg1;
                    check_1st=true;
                } else if (line_type == "newmtl"){
                    if (!curr_name.empty() &&
                        materials.count(curr_name) == 0)
                        materials.insert({curr_name, curr_material});
                    material curr_material;
                    curr_name = str_arg1;
                } else if (line_type == "map_Kd") { //hope the texture's in the same path 
                    std::vector<std::string> split_path = split(path, "/");
                    split_path.pop_back();
                    split_path.push_back(str_arg1);
                    curr_material.texture_path = join(split_path, "/");
                }
            } catch (std::invalid_argument) {
                std::cerr << "Error in file " << path << " at line " << line_num << '\n';
                return {};
            }
            bool failure=false;
            if ((check_1st || check_rest) && std::count(str_arg1.begin(), str_arg1.end(), '.') > 1)
                failure=true;
            if (check_rest && std::count(str_arg2.begin(), str_arg2.end(), '.') > 1)
                failure=true;
            if (check_rest && std::count(str_arg3.begin(), str_arg3.end(), '.') > 1)
                failure=true;
            if (failure){
                std::cerr << "Error in file " << path << " at line " << line_num << '\n';
                return {};
            }
        }
        if (!curr_name.empty() &&
            materials.count(curr_name) == 0)
            materials.insert({curr_name, curr_material});

        return materials;
    }
    //if it fails it returns an empty vector
    //pass it an std::string specifying path and it returns meshes
    std::vector<mesh> loader(const std::string& path, int thread_num)
    {
        std::vector<glm::vec3> vert_data, normal_data;
        std::vector<glm::vec2> uv_coord_data;
        std::unordered_map<std::string, material> materials;
        std::vector<mesh> groups;
        mesh curr_group;
        std::ifstream obj_file;
        int vertex_offset = 0, uv_offset = 0, normal_offset = 0; //multiple objects; say if there are 8 vertices in the first and 500 in the second, to access the last in the second, it would use f 508/../..
        obj_file.open(path);
        if (obj_file.fail()) {
            std::cerr << "Error opening file: " << std::strerror(errno) << '\n';
            return {};
        }
        std::string line="", line_type = "";
        std::stringstream line_stream;
        int line_num = 0, face_count = 0;
        mesh empty_mesh;
        int curr_thread = 0;
        std::vector<std::string> threads_data[thread_num];
        long long v_time=0, uv_time=0, vn_time=0, f_time=0, o_time=0, mtl_time=0, usemtl_time=0; 
        for (;getline(obj_file, line);)
        {
            bool check3rd = false, checkfloats = false;
            float arg1=FLT_INF, arg2=FLT_INF, arg3=FLT_INF;
            line_stream = std::stringstream(line);
            std::string str_arg1, str_arg2, str_arg3;
            line_stream >> line_type >> str_arg1 >> str_arg2 >> str_arg3;
//            std::cout << line_type << ' ' << str_arg1 << ' ' << str_arg2 << ' ' << str_arg3 << '\n';
            try { arg1 = std::stof(str_arg1); } 
            catch (std::invalid_argument) {}
            try { arg2 = std::stof(str_arg2); } 
            catch (std::invalid_argument) {}
            try { arg3 = std::stof(str_arg3); } 
            catch (std::invalid_argument) {}
            //stupid but i don't want to add another function to validate floats; should be okay
            if (std::count(str_arg1.begin(), str_arg1.end(), '.') > 1)
                arg1 = FLT_INF;
            if (std::count(str_arg2.begin(), str_arg2.end(), '.') > 1)
                arg2 = FLT_INF;
            if (std::count(str_arg3.begin(), str_arg3.end(), '.') > 1)
                arg3 = FLT_INF;
            line_num += 1;
            if (line_type == "v") { //vertex
                vert_data.push_back(glm::vec3(arg1, arg2, arg3));
                check3rd = true;
            } else if (line_type == "vt") { //uvs
                uv_coord_data.push_back(glm::vec2(arg1, arg2));
                checkfloats = true;
            } else if (line_type == "vn") { //normals
                normal_data.push_back(glm::vec3(arg1, arg2, arg3));
                check3rd = true;
            } else if (line_type == "f") { //face
                face_count += 1;
                threads_data[curr_thread].push_back(line);
                curr_thread += 1;
                curr_thread %= thread_num;
            } else if (line_type == "o") { //new objects
                if (face_count > 0) {
                    std::future<std::tuple<std::vector<point>, std::vector<std::array<int, 3>>>> threads[thread_num];
                    std::vector<std::tuple<std::vector<point>, std::vector<std::array<int, 3>>>> output;
                    for (int i=0; i<thread_num; i++) {
                        threads[i] = std::async(process_faces, std::ref(threads_data[i]), std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), line_num, vertex_offset, uv_offset, normal_offset);
                    }
                    for (int i=0; i<thread_num; i++)
                        threads[i].wait();
                    for (int i=0; i<thread_num; i++)
                        output.push_back(threads[i].get());
                    int point_count = 0;
                    std::vector<point> mesh_points;
                    std::vector<std::array<int, 3>> point_indexes;
                    for (auto num : output){
                        mesh_points.insert(mesh_points.end(), std::get<0>(num).begin(), std::get<0>(num).end());
                        for (std::array<int, 3> triangle : std::get<1>(num))
                        {
                            for (int i=0; i<3; i++)
                            {
                                triangle[i] += point_count;
                            }
                            point_indexes.push_back(triangle);
                        }
                        point_count += std::get<0>(num).size();
                    }
                    curr_group.data.insert(curr_group.data.end(), mesh_points.begin(), mesh_points.end());
                    curr_group.indices.insert(curr_group.indices.end(), point_indexes.begin(), point_indexes.end());
                    groups.push_back(curr_group);
                }
                mesh new_mesh; //clear the current mesh
                curr_group = new_mesh;
                vertex_offset += vert_data.size();
                uv_offset += uv_coord_data.size();
                normal_offset += normal_data.size();
                vert_data.clear(); uv_coord_data.clear(); normal_data.clear();// clear vectors
                for (int i=0; i<thread_num; i++)
                    threads_data[i].clear();
                curr_group.group_name = str_arg1;
                std::cout << curr_group.group_name << '\n';
                face_count = 0;
            } else if (line_type == "mtllib") { // hope you used global paths
                std::vector<std::string> split_path = split(path, std::string("/"));
                split_path.pop_back();
                split_path.push_back(str_arg1);
                materials = load_mtl(join(split_path, "/"));
            } else if (line_type == "usemtl") {
                if (materials.count(str_arg1) != 0) {
                    curr_group.used_mtl = materials[str_arg1];
                } else {
                    std::cerr << "Material " << str_arg1 << " was not defined\n";
                    return {};
                }
            }
            //being FLT_INF means that either std::stof failed or it had more than one decimal point
            if ((arg1 == FLT_INF || arg2 == FLT_INF) && (check3rd || checkfloats)) {
                std::cerr << "Error in .obj file at line " << line_num << "\n";
                return {};
            }
            if (check3rd && arg3 == FLT_INF) {
                std::cerr << "Error in file at line " << line_num << "\n";
                return {};
            }
        }
        std::future<std::tuple<std::vector<point>, std::vector<std::array<int, 3>>>> threads[thread_num];
        std::vector<std::tuple<std::vector<point>, std::vector<std::array<int, 3>>>> output;
        for (int i=0; i<thread_num; i++) {
            threads[i] = std::async(process_faces, std::ref(threads_data[i]), std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), line_num, vertex_offset, uv_offset, normal_offset);
        }
        for (int i=0; i<thread_num; i++)
            threads[i].wait();
        for (int i=0; i<thread_num; i++)
            output.push_back(threads[i].get());
        int point_count = 0;
        std::vector<point> mesh_points;
        std::vector<std::array<int, 3>> point_indexes;
        for (auto num : output){
            mesh_points.insert(mesh_points.end(), std::get<0>(num).begin(), std::get<0>(num).end());
            for (std::array<int, 3> triangle : std::get<1>(num))
            {
                for (int i=0; i<3; i++)
                {
                    triangle[i] += point_count;
                }
                point_indexes.push_back(triangle);
            }
            point_count += std::get<0>(num).size();
        }
        curr_group.data.insert(curr_group.data.end(), mesh_points.begin(), mesh_points.end());
       curr_group.indices.insert(curr_group.indices.end(), point_indexes.begin(), point_indexes.end());
        groups.push_back(curr_group);
        vert_data.clear(); uv_coord_data.clear(); normal_data.clear();
        return groups;
    }
}

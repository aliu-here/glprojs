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
#include "triangulate.hpp"


namespace loader
{ 
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
    std::vector<mesh> loader(const std::string& path, int thread_num=50)
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
        int vert_count=0;
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
                int v_last_idx = vert_data.size() - 1;
                vert_count++;
                for (int i=0; i<3; i++)
                {
                    curr_group.bounding_box.min[i] = std::min(curr_group.bounding_box.min[i], vert_data[v_last_idx][i]);
                    curr_group.bounding_box.max[i] = std::max(curr_group.bounding_box.max[i], vert_data[v_last_idx][i]);
                }
                //39106//51322 62097//51324 74710//51325 62103//51323
                if (vert_count == 39106 || vert_count == 62097 || vert_count == 74710 || vert_count == 51323) {
                    print_vec3(vert_data[vert_data.size() - 1]);
                    std::cout << line << '\n';
                    std::cout << "\n";
                }
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

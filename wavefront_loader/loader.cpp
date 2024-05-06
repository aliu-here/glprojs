#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <array>
#include <tuple>
#include <atomic>
#include <thread>
#include <chrono>

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
                    curr_material = material();
                    curr_name = str_arg1;
                    curr_material.name = str_arg1;
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

    //calls the triangulation functions; packaged into a function for parallelization
    //it's intended to be given all faces, but it works because we're going from
    //top down in a file; so it's not going to use anything that doesn't exist yet
    void threaded_triangulate(mesh group, const std::vector<std::string> *allfaces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, int line_num, std::atomic<unsigned int>& done_flag, std::vector<mesh>& out_groups, std::atomic<bool>& vec_used) {
        std::tuple<std::vector<point>, std::vector<std::array<int, 3>>> processed_out = process_faces(*allfaces, coords, tex, normals, line_num);
        group.data = std::get<0>(processed_out);
        group.indices = std::get<1>(processed_out);

        delete allfaces;

        while (vec_used)
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); //dont destroy cpu usage; check every ms instead of every cycle
        
        vec_used.store(true);
        out_groups.push_back(group);
        vec_used.store(false);

        done_flag.fetch_add(1);
    }
    
    //if it fails it returns an empty vector
    //pass it an std::string specifying path and it returns meshes
    std::vector<mesh> loader(const std::string& path, bool usemt=true)
    {
        std::atomic<bool> used = false;
        std::atomic<unsigned int> finished = 0;

        std::vector<glm::vec3> vert_data, normal_data;
        std::vector<glm::vec2> uv_coord_data;
        std::vector<std::vector<std::string>> faces_per_group;
        std::vector<std::string> *curr_group_lines = new std::vector<std::string>; //to each their own; so the vec doesn't get overwritten when another one uses it
        std::unordered_map<std::string, material> materials;
        std::vector<mesh> groups;
        std::vector<int> line_nums;
        mesh curr_group;
        std::ifstream obj_file;
        obj_file.open(path);
        if (obj_file.fail()) {
            std::cerr << "Error opening file: " << std::strerror(errno) << '\n';
            return {};
        }
        std::string line="", line_type = "";
        std::stringstream line_stream;
        int line_num = 0, face_count = 0, line_num_for_triangulate = 0, ocount = 0;
        mesh empty_mesh;
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
            line_num++;
            if (line_type == "v") { //vertex
                vert_data.push_back(glm::vec3(arg1, arg2, arg3));
                int v_last_idx = vert_data.size() - 1;
                vert_count++;
                for (int i=0; i<3; i++)
                {
                    curr_group.bounding_box.min[i] = std::min(curr_group.bounding_box.min[i], vert_data[v_last_idx][i]);
                    curr_group.bounding_box.max[i] = std::max(curr_group.bounding_box.max[i], vert_data[v_last_idx][i]);
                }
                check3rd = true;
            } else if (line_type == "vt") { //uvs
                uv_coord_data.push_back(glm::vec2(arg1, arg2));
                checkfloats = true;
            } else if (line_type == "vn") { //normals
                normal_data.push_back(glm::vec3(arg1, arg2, arg3));
                check3rd = true;
            } else if (line_type == "f") { //face
                face_count++;
                curr_group_lines->push_back(line);
            } else if (line_type == "o") { //new objects
                if (face_count > 0) {
                    std::cout << curr_group.group_name << '\n';
                    for (auto x : *curr_group_lines)
                        std::cout << "loader::loader - " << x << '\n';

                    if (usemt) {
                        std::thread temp(threaded_triangulate, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), line_num_for_triangulate, std::ref(finished), std::ref(groups), std::ref(used));
                        temp.detach();
                        ocount++;
                    } else {
                        auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data, line_num_for_triangulate);
                        curr_group.data = std::get<0>(temp);
                        curr_group.indices = std::get<1>(temp);
                        groups.push_back(curr_group);
                    }
                }
                if (!usemt || face_count <= 0)
                    delete curr_group_lines;

                mesh new_mesh; //clear the current mesh
                curr_group = mesh();
                curr_group.group_name = str_arg1;
                if (groups.size() > 0)
                    curr_group.used_mtl = groups[groups.size() - 1].used_mtl;
                std::cout << curr_group.group_name << '\n';

                curr_group_lines = new std::vector<std::string>;
                face_count = 0;
                line_num_for_triangulate = line_num;
            } else if (line_type == "mtllib") { // hope you used global paths
                std::vector<std::string> split_path = split(path, std::string("/"));
                split_path.pop_back();
                split_path.push_back(str_arg1);
                materials = load_mtl(join(split_path, "/"));
            } else if (line_type == "usemtl") {
                if (materials.count(str_arg1) != 0) {
                    std::cout << "material " << str_arg1 << '\n';
                    if (curr_group.used_mtl.name.size() != 0) { //check if there's already a material defined
                        std::cout << face_count << '\n';
                        if (face_count > 0) {
                            for (auto x : *curr_group_lines)
                                std::cout << "loader::loader - " << x << '\n';
                            if (usemt) {
                                std::thread temp(threaded_triangulate, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), line_num_for_triangulate, std::ref(finished), std::ref(groups), std::ref(used));
                                temp.detach();
                                ocount++;
                            } else {
                                auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data, line_num_for_triangulate);
                                curr_group.data = std::get<0>(temp);
                                curr_group.indices = std::get<1>(temp);
                                groups.push_back(curr_group);
                            }
                            if (!usemt || face_count <= 0)
                                delete curr_group_lines;
                        }

                        mesh new_mesh; //clear the current mesh
                        curr_group = mesh();
                        curr_group.used_mtl = materials[str_arg1];
                        curr_group.group_name = str_arg1; //so its easier to debug
                        std::cout << curr_group.group_name << '\n';

                        curr_group_lines = new std::vector<std::string>; //new pointer
                        face_count = 0;
                        line_num_for_triangulate = line_num;
                    }
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
        std::cout << "face_count=" << face_count << '\n';
        if (face_count > 0) {
            for (auto x : *curr_group_lines)
                std::cout << "loader::loader - " << x << '\n';

            if (usemt) {
                std::thread temp(threaded_triangulate, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), line_num_for_triangulate, std::ref(finished), std::ref(groups), std::ref(used));
                temp.detach();
                ocount++;
            } else {
                auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data, line_num_for_triangulate);
                curr_group.data = std::get<0>(temp);
                curr_group.indices = std::get<1>(temp);
                groups.push_back(curr_group);
            }
        }
        if (!usemt || face_count <= 0)
            delete curr_group_lines;
        std::cout << "ocount=" << ocount << '\n';

        if (usemt) {
            while (finished != ocount) 
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        return groups;
    }
}

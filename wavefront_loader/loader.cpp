#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <array>
#include <tuple>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

#include "mesh.hpp"
#include "string_utils.hpp"
#include "triangulate.hpp"

namespace loader
{ 
    using namespace std::chrono_literals;

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
        std::string line;
        while (getline(mtl_file, line))
        {
            bool check_1st=false, check_rest=false;
            line_num++;
            std::string_view str_arg1 = "", str_arg2 = "", str_arg3 = "", line_type = "";
            float arg1=NAN, arg2=NAN, arg3=NAN;
            std::vector<std::string_view> split_line = split(line, " ");

            line_type = split_line[0];
            if (split_line.size() > 1)
                str_arg1 = split_line[1];
            if (split_line.size() > 2)
                str_arg2 = split_line[2];
            if (split_line.size() > 3)
                str_arg3 = split_line[3];

            if (line_type[0] == 'K') {
                try { arg1 = to_float(str_arg1); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 1 in file " << path << " line " << line_num << '\n';
                }
                try { arg2 = to_float(str_arg2); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 2 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg3 = to_float(str_arg3); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 3 in file " << path << " line " << line_num << '\n'; 
                }

                glm::vec3 curr_color = glm::vec3(arg1, arg2, arg3);
                check_rest=true;
                if (line_type == "Ka")
                    curr_material.ambient = curr_color;
                else if (line_type == "Kd")
                    curr_material.diffuse = curr_color;
                else if (line_type == "Ks")
                    curr_material.specular = curr_color;
            } else if (line_type == "Ns") {
                try { arg1 = to_float(str_arg1); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 1 in file " << path << " line " << line_num << '\n'; 
                }

                curr_material.specular_exp = arg1;
                check_1st=true;
            } else if (line_type == "newmtl"){
                std::cout << "name: " << curr_name << '\n';
                std::cout << (materials.find(curr_name) == materials.end()) << '\n';
                std::cout << (curr_name.size() != 0) << '\n';
                if (curr_name.size() != 0 &&
                    materials.find(curr_name) == materials.end())
                    materials[curr_name] = curr_material;

                curr_material = material();
                curr_name = str_arg1;
                curr_material.name = str_arg1;
            } else if (line_type == "map_Kd") { //hope the texture's in the same path 
                std::vector<std::string_view> split_path = split(path, "/");
                split_path.pop_back();
                split_path.push_back(str_arg1);
                curr_material.texture_path = join(split_path, "/");
            }
        }
        std::cout << "name: " << curr_name << '\n';
        if (!curr_name.empty() &&
            materials.find(curr_name) == materials.end())
            materials[curr_name] = curr_material;

        for (auto pair : materials)
            std::cout << "material name: " << pair.first << '\n';
        return materials;
    }

    void triangulate_worker(std::vector<std::string>& assignedfaces, const std::vector<glm::vec3>& verts, const std::vector<glm::vec2>& uvcoords, const std::vector<glm::vec3>& normals, std::vector<std::tuple<std::vector<point>*, std::vector<std::array<unsigned int,3>>*>*>& outvec, std::atomic<bool>& outvec_used, std::atomic<unsigned int>& finished, int id) {
        std::tuple<std::vector<point>*, std::vector<std::array<unsigned int, 3>>*> *processed_out = process_faces(assignedfaces, verts, uvcoords, normals);

        while (outvec_used)
            std::this_thread::sleep_for(1ms);

        outvec[id] = processed_out;

        finished.fetch_add(1);
    }

    //calls the triangulation functions; packaged into a function for parallelization
    void threaded_triangulate_boss(mesh group, std::vector<std::string> *allfaces, const std::vector<glm::vec3>& coords, const std::vector<glm::vec2>& tex, const std::vector<glm::vec3>& normals, std::atomic<unsigned int>& done_flag, std::vector<mesh>& out_groups, std::atomic<bool>& vec_used, int thread_count) {

        std::vector<std::tuple<std::vector<point>*, std::vector<std::array<unsigned int, 3>>*>*> worker_output(thread_count);

        std::atomic<unsigned int> worker_finished = 0;
        std::atomic<bool> using_outvec = false;

        std::vector<std::vector<std::string>> dist_work = std::vector<std::vector<std::string>>(thread_count);
        for (int i=0; i<allfaces->size(); i++) {
            dist_work[i % thread_count].push_back((*allfaces)[i]);
        }
        for (int i=0; i<thread_count; i++) {
            std::thread worker(triangulate_worker, std::ref(dist_work[i]), std::ref(coords), std::ref(tex), std::ref(normals), std::ref(worker_output), std::ref(using_outvec), std::ref(worker_finished), i);
            worker.detach();
        }


        while (worker_finished != thread_count)
            std::this_thread::sleep_for(1ms);


        delete allfaces;

        group.data.clear();
        group.indices.clear();

        int totalpointcount = 0;
        for (auto out : worker_output) {
            group.data.insert(group.data.end(), std::get<0>(*out)->begin(), std::get<0>(*out)->end());
            for (int i=0; i<std::get<1>(*out)->size(); i++)
                for (int j=0; j<3; j++)
                    (*std::get<1>(*out))[i][j] += totalpointcount;
            group.indices.insert(group.indices.end(), std::get<1>(*out)->begin(), std::get<1>(*out)->end());

            totalpointcount = group.data.size();

            delete std::get<0>(*out);
            delete std::get<1>(*out);
            delete out;
        }

        while (vec_used)
            std::this_thread::sleep_for(1ms); //dont destroy cpu usage; check every ms instead of every cycle
        
        vec_used.store(true);
        out_groups.push_back(group);
        vec_used.store(false);

        done_flag.fetch_add(1);
    }
    
    //if it fails it returns an empty vector
    //pass it an std::string specifying path and it returns meshes
    std::vector<mesh> loader(const std::string& path, bool usemt = true, int thread_count = 8)
    {
        using namespace std::chrono_literals;
        std::cout << "loader::loader called\n";

        std::atomic<bool> used = false;
        std::atomic<unsigned int> finished = 0;

        std::ifstream obj_file(path);
        if (obj_file.fail()) {
            std::cerr << "Error opening file: " << std::strerror(errno) << '\n';
            return {};
        }

        std::vector<glm::vec3> vert_data, normal_data;
        std::vector<glm::vec2> uv_coord_data;
        std::vector<std::vector<std::string>> faces_per_group;
        std::unique_ptr<std::vector<std::string>> curr_group_lines = std::make_unique<std::vector<std::string>>(); //to each their own; so the vec doesn't get overwritten when another one uses it
        std::unordered_map<std::string, material> materials;
        std::vector<mesh> groups;
        std::vector<int> line_nums;
        mesh curr_group;

        std::string line="";
        int line_num = 0, face_count = 0, line_num_for_triangulate = 0, ocount = 0;
        mesh empty_mesh;
        int vert_count=0;
        while (getline(obj_file, line))
        {
            float arg1=FLT_INF, arg2=FLT_INF, arg3=FLT_INF;
            std::string_view str_arg1, str_arg2, str_arg3, line_type;
            std::vector<std::string_view> split_line = split(line, " ");
            str_arg1 = split_line[1];
            str_arg2 = split_line[2];
            str_arg3 = split_line[3];
            line_type = split_line[0];

            line_num++;
            if (line_type == "v") { //vertex
                try { arg1 = to_float(str_arg1); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 1 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg2 = to_float(str_arg2); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 2 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg3 = to_float(str_arg3); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 3 in file " << path << " line " << line_num << '\n'; 
                }

                vert_data.push_back(glm::vec3(arg1, arg2, arg3));
                int v_last_idx = vert_data.size() - 1;
                vert_count++;
                for (int i=0; i<3; i++)
                {
                    curr_group.bounding_box.min[i] = std::min(curr_group.bounding_box.min[i], vert_data[v_last_idx][i]);
                    curr_group.bounding_box.max[i] = std::max(curr_group.bounding_box.max[i], vert_data[v_last_idx][i]);
                }
            } else if (line_type == "vt") { //uvs
                try { arg1 = to_float(str_arg1); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 1 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg2 = to_float(str_arg2); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 2 in file " << path << " line " << line_num << '\n'; 
                }
                uv_coord_data.push_back(glm::vec2(arg1, arg2));
            } else if (line_type == "vn") { //normals
                try { arg1 = to_float(str_arg1); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 1 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg2 = to_float(str_arg2); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 2 in file " << path << " line " << line_num << '\n'; 
                }
                try { arg3 = to_float(str_arg3); } 
                catch (std::invalid_argument) {
                    std::cerr << "loader::load_mtl: error in float argument 3 in file " << path << " line " << line_num << '\n'; 
                }
                normal_data.push_back(glm::vec3(arg1, arg2, arg3));
            } else if (line_type == "f") { //face
                face_count++;
                curr_group_lines->push_back(line);
            } else if (line_type == "o") { //new objects
                if (face_count > 0) {

                    if (usemt) {
                        std::thread temp(threaded_triangulate_boss, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), std::ref(finished), std::ref(groups), std::ref(used), thread_count);
                        temp.detach();
                        ocount++;
                    } else {
                        auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data);
                        curr_group.data = *std::get<0>(*temp);
                        curr_group.indices = *std::get<1>(*temp);
                        groups.push_back(curr_group);

                        delete std::get<0>(*temp);
                        delete std::get<1>(*temp);
                        delete temp;
                    }
                }

                mesh new_mesh; //clear the current mesh
                curr_group = mesh();
                curr_group.group_name = str_arg1;
                if (groups.size() > 0)
                    curr_group.used_mtl = groups[groups.size() - 1].used_mtl;

                curr_group_lines = std::make_unique<std::vector<std::string>>();
                face_count = 0;
            } else if (line_type == "mtllib") { // hope you used global paths
                std::vector<std::string_view> split_path = split(path, std::string("/"));
                split_path.pop_back();
                split_path.push_back(str_arg1);
                std::cout << "loader::loader: file name: " << join(split_path, "/") << '\n';
                materials = load_mtl(join(split_path, "/"));
            } else if (line_type == "usemtl") {
                if (materials.find(str_arg1.data()) != materials.end()) {
                    if (curr_group.used_mtl.name.size() != 0) { //check if there's already a material defined; kind of stupid but oh well
                        if (face_count > 0) {
                            if (usemt) {
                                std::thread temp(threaded_triangulate_boss, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), std::ref(finished), std::ref(groups), std::ref(used), thread_count);
                                temp.detach();
                                ocount++;
                            } else {
                                auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data);
                                curr_group.data = *std::get<0>(*temp);
                                curr_group.indices = *std::get<1>(*temp);
                                groups.push_back(curr_group);

                                delete std::get<0>(*temp);
                                delete std::get<1>(*temp);
                                delete temp;
                            }
                        }

                        mesh new_mesh; //clear the current mesh
                        curr_group = mesh();
                        curr_group.used_mtl = materials[str_arg1.data()];
                        curr_group.group_name = str_arg1; //so its easier to debug

                        curr_group_lines = std::make_unique<std::vector<std::string>>(); //new pointer
                        face_count = 0;
                    }
                    curr_group.used_mtl = materials[str_arg1.data()];
                } else {
                    std::cerr << "Material " << str_arg1 << " was not defined\n";
                    goto exit_on_failure;
                }
            }
        }
        if (face_count > 0) {
            if (usemt) {
                std::thread temp(threaded_triangulate_boss, curr_group, curr_group_lines, std::ref(vert_data), std::ref(uv_coord_data), std::ref(normal_data), std::ref(finished), std::ref(groups), std::ref(used), thread_count);
                temp.detach();
                ocount++;
            } else {
                auto temp = process_faces(*curr_group_lines, vert_data, uv_coord_data, normal_data);
                curr_group.data = *std::get<0>(*temp);
                curr_group.indices = *std::get<1>(*temp);
                groups.push_back(curr_group);

                delete std::get<0>(*temp);
                delete std::get<1>(*temp); //this is ass bc i have to repeat this exact segment like five times
                                           //oh well
                delete temp;
            }
        }
        if (usemt) {
            while (finished != ocount)
                std::this_thread::sleep_for(1ms);
        }

        std::cout << "loader::loader finished successfully\n";
        return groups;

        exit_on_failure:
        std::cout << "loader::loader failed\n";
        return {};
    }
}

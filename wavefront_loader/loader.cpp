#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <limits>
#include "mesh.hpp"
#include "string_utils.hpp"


const float FLT_INF = std::numeric_limits<float>::infinity();

namespace loader
{
	//this only works if the user gives the correct path 
	//loads a material, stores ambient, diffuse, specular as glm::vec3 and stores specular_exponent as float
	std::unordered_map<std::string, material> load_mtl(const std::string& path)
	{
		std::unordered_map<std::string, material> materials;
		std::vector<std::string> names;
		bool check_1st=false, check_rest=false;
		material curr_material;
		std::ifstream mtl_file;
		mtl_file.open(path);
		if (mtl_file.fail())
		{
			std::cerr << "Failed to open material file\n";
			return materials;
		}
		std::string line, curr_name;
		int line_num=0; //just to say where it went wrong
		while (mtl_file)
		{
			std::getline(mtl_file, line);
			line_num++;
			std::vector<std::string> split_line = split(line, " ");
			try {
				if (split_line[0][0] == 'K') {
					glm::vec3 curr_color = glm::vec3(std::stof(split_line[1]), std::stof(split_line[2]), std::stof(split_line[3]));
					check_rest=true;
					if (split_line[0] == "Ka")
						curr_material.ambient = curr_color;
					else if (split_line[0] == "Kd")
						curr_material.diffuse = curr_color;
					else if (split_line[0] == "Ks")
						curr_material.specular = curr_color;
				} else if (split_line[0] == "Ns") {
					curr_material.specular_exp = std::stof(split_line[1]);
					check_1st=true;
				} else if (split_line[0] == "newmtl"){
					if (!curr_name.empty() &&
						materials.count(curr_name) == 0)
						materials.insert({curr_name, curr_material});
					material curr_material;
					curr_name = split_line[1];
				} else if (split_line[0] == "map_Kd") { //hope the texture's in the same path 
					std::vector<std::string> split_path = split(path, "/");
					split_path.pop_back();
					split_path.push_back(split_line[1]);
					curr_material.texture_path = join(split_path, "/");
				}
			} catch (std::invalid_argument) {
				std::cerr << "Error in file " << path << " at line " << line_num << '\n';
				return {};
			}
			bool failure=false;
			if ((check_1st || check_rest) && std::count(split_line[1].begin(), split_line[1].end(), '.') > 1)
				failure=true;
			if (check_rest && std::count(split_line[2].begin(), split_line[2].end(), '.') > 1)
				failure=true;
			if (check_rest && std::count(split_line[2].begin(), split_line[2].end(), '.') > 1)
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
	//generates triangles based on a 'face' line of the .obj file; triangles are point[3]
	std::vector<triangle> generate_triangles(const std::vector<std::string>& data, int line_num, const std::vector<std::vector<float>>& vert_data, const std::vector<std::vector<float>>& uv_coord_data, const std::vector<std::vector<float>>& normal_data, int vertex_offset, int uv_offset, int normal_offset)
	{
		std::vector<triangle> output;
		bool include_tex_data, include_normal_data;
		for (int it=1; it<data.size() - 1; it++)
		{
			triangle temp;
			int indices[3] = {0, it, it+1};
			bool error_detected = false;
			for (int curr_point=0; curr_point < 3; curr_point++)
			{
//				std::cout << "before converting split data to int\n";
				std::vector<std::string> split_data = split(data[indices[curr_point]], std::string("/"));
				std::vector<int> locs;
				for (std::string num: split_data)
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
				for (int i=0; i<8; i++) { temp.points[curr_point].data[i] = 0.0f; }
				if (locs.size() < 1 || locs.size() > 3 || error_detected == true)
					std::cerr << "Error in .obj file at line " << line_num << '\n';
				else {
//					std::cout << "adding vertex data\n";
					if (locs[0] < 0)
						locs[0] += vert_data.size(); //.obj allows negative numbers; from back
					else 
						locs[0] -= 1 + vertex_offset; //.obj is 1-indexed
//					std::cout << locs[0] << '\n';
//					std::cout << vert_data.size() << '\n';
//					std::cout << join(data, " ") << '\n';
					for (int i=0; i<3; i++) {
						temp.points[curr_point].data[i] = vert_data[locs[0]][i];
					}
				}
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
//				std::cout << "adding uvs\n";
				if (include_tex_data) {
					if (locs[1] < 0)
						locs[1] += uv_coord_data.size();
					else 
						locs[1] -= 1 + uv_offset;
					for (int i=0; i<2; i++)
					{
						temp.points[curr_point].data[i + 3] = uv_coord_data[locs[1]][i];
					}
				} if (include_normal_data) {
//					std::cout << "adding normals\n";
					if (locs[2] < 0)
						locs[2] += normal_data.size();
					else 
						locs[2] -= 1 + normal_offset;
//					std::cout << locs[2] << '\n';
//					std::cout << normal_data.size() << '\n';
//					std::cout << curr_point << '\n';
					for(int i=0; i<3; i++) 
					{
//						std::cout << "loop" << '\n';
						temp.points[curr_point].data[i + 5] = normal_data[locs[2]][i];
					}
				}
			}
		}
		return output;
	}
	//if it fails it returns an empty vector
	//pass it an std::string specifying path and it returns meshes
	std::vector<mesh> loader(const std::string& path)
	{
		std::vector<std::vector<float>> vert_data, uv_coord_data, normal_data;
		std::unordered_map<std::string, material> materials;
		std::vector<mesh> groups;
		mesh curr_group;
		std::ifstream obj_file;
		int vertex_offset = 0, uv_offset = 0, normal_offset = 0; //multiple objects; say if there are 8 vertices in the first and 500 in the second, to access the last in the second, it would use f 508/../..
		obj_file.open(path);
		if (obj_file.fail())
		{
			std::cerr << "Error opening file: " << std::strerror(errno) << '\n';
		} else {
			std::string line, line_type = "", face_args, str_arg1, str_arg2, str_arg3;
			std::stringstream line_stream;
			int line_num = 0, face_count = 0;
			mesh empty_mesh;
			for (line ; getline(obj_file, line);)
			{
				bool check3rd = false, checkfloats = false;
				float arg1=FLT_INF, arg2=FLT_INF, arg3=FLT_INF;
				line_stream = std::stringstream(line);
				line_stream >> line_type >> str_arg1 >> str_arg2 >> str_arg3;
//				std::cout << line_type << ' ' << str_arg1 << ' ' << str_arg2 << ' ' << str_arg3 << '\n';
				try{ 
					arg1 = std::stof(str_arg1);
				} catch (std::invalid_argument) {}
				try { 
					arg2=std::stof(str_arg2);
				} catch (std::invalid_argument) {}
				try {
					arg3=std::stof(str_arg3);
				} catch (std::invalid_argument) {}
				//stupid but i don't want to add another function to validate floats; should be okay
				if (std::count(str_arg1.begin(), str_arg1.end(), '.') > 1)
					arg1 = FLT_INF;
				if (std::count(str_arg2.begin(), str_arg2.end(), '.') > 1)
					arg2 = FLT_INF;
				if (std::count(str_arg3.begin(), str_arg3.end(), '.') > 1)
					arg3 = FLT_INF;
				line_num += 1;
				if (line_type == "v") { //vertex
					vert_data.push_back({arg1, arg2, arg3});
					check3rd = true;
				} else if (line_type == "vt") { //uvs
					uv_coord_data.push_back({arg1, arg2});
					checkfloats = true;
				} else if (line_type == "vn") { //normals
					normal_data.push_back({arg1, arg2, arg3});
					check3rd = true;
				} else if (line_type == "f") { //face
					std::vector<std::string> split_lines = split(line, " ");
					std::vector<std::string> temp(split_lines.begin() + 1, split_lines.end());
					std::vector<triangle> temp_triangles = generate_triangles(temp, line_num, vert_data, uv_coord_data, normal_data, vertex_offset, uv_offset, normal_offset);
					curr_group.data.insert(curr_group.data.end(), temp_triangles.begin(), temp_triangles.end());
					face_count += 1;
				} else if (line_type == "o") { //new object
					if (face_count == 0)
						continue;
					groups.push_back(curr_group);
					mesh curr_group; //clear the current mesh
					vertex_offset += vert_data.size();
					uv_offset += uv_coord_data.size();
					normal_offset += normal_data.size();
					vert_data.clear(); uv_coord_data.clear(); normal_data.clear(); // clear vectors
					curr_group.group_name = str_arg1;
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
			groups.push_back(curr_group);
			vert_data.clear(); uv_coord_data.clear(); normal_data.clear();
		}
		return groups;
	}
}

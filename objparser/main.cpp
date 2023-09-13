#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

class objData
{
	public:
	std::vector<glm::vec3> points;
	std::vector<glm::vec2> coords;
	std::vector<glm::vec3> normals;
	std::vector<std::vector<int>> connects;
	std::vector<std::string> objLines; 
	std::vector<std::string> mtlLines;
	std::string diffusePath;
	std::string ambientPath;
	std::string specularPath;
	std::string alphaMap;
	
	std::vector<float> output;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float specularExp;
	float Transparency;
	glm::vec3 TFcolor;

	objData(std::string filePath)
	{
		std::string objOutput = openFile(filePath);
  		std::vector<std::string> objLines = split(objOutput, '\n');
		processData();
	}
	
	void processMtl(std::string path)
	{
		std::string mtlOutput = openFile(path);
		std::vector<std::string> mtlLines = split(mtlOutput, '\n');
		for (int i=0; i<mtlLines.size(); i++)
		{
			std::vector<std::string> args = split(mtlLines[i], ' ');
			if (args[0] == "Ns")
			{
				specularExp = stof(args[1]);
			}
			if (args[0] == "Ka")
			{
				ambient = glm::vec3(stof(args[1]), stof(args[2]), stof(args[3]));
			}
			if (args[0] == "Kd")
			{
				diffuse = glm::vec3(stof(args[1]), stof(args[2]), stof(args[3]));
			}
			if (args[0] == "Ks")
			{
				specular = glm::vec3(stof(args[1]), stof(args[2]), stof(args[3]));
			}
			if (args[0] == "d")
			{
				Transparency = stof(args[1]);
			}
			if (args[0] == "Tr")
			{
				Transparency = 1 - stof(args[1]);
			}
			if (args[0] == "Tf")
			{
				TFcolor = glm::vec3(stof(args[1]), stof(args[2]), stof(args[3]));
			}
		}
	}

	void processF(std::vector<std::string> args)
	{
		for (int i=1; i<args.size()-1; i++)
		{
			std::vector<std::string> point1 = split(args[i], '/');
			std::vector<std::string> point2 = split(args[i + 1], '/');
		}
	}
	
	void processData()
	{
		std::vector<std::string> possibleLineStarts = {"v", "vt", "vn", "f", "mtllib"};
		for (int i=0; i<objLines.size(); i++)
		{
			std::vector<std::string> args = split(objLines[i], ' ');
			if (args[0] == "v")
			{
				points.push_back(glm::vec3(stof(args[1]), stof(args[2]), stof(args[3])));
			}
			if (args[0] == "vt")
			{
				coords.push_back(glm::vec2(stof(args[1]), stof(args[2])));
			}
			if (args[0] == "vn")
			{
				normals.push_back(glm::vec3(stof(args[1]), stof(args[2]), stof(args[3])));
			}
			if (args[0] == "mtllib")
			{
				processMtl(args[1]);
			}
			if (args[0] == "f")
			{
				processF(args);
			}
		}
	}

	private:
	std::vector<std::string> split (const std::string &s, char delim) 
	{
    		std::vector<std::string> result;
    		std::stringstream ss (s);
    		std::string item;

    		while (getline (ss, item, delim)) {
        		result.push_back (item);
		}
		return result;
	}

	std::string openFile(const std::string filePath)
	{		
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		std::string contents;
		if (in)
		{
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return contents;
  		} else {
			std::cout << "invalid path\n";
			throw(errno);
  		}

	}
};


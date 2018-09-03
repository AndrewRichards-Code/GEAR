#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "typesx86x64.h"
#include "../maths/ARMLib.h"

namespace GEAR {

class FileUtils 
{
public:
	static std::string read_file(const char* filepath)
	{
		std::ifstream stream(filepath, std::fstream::in);
		std::string output;

		if (!stream.is_open())
		{
			std::cout << "Could not read file " << filepath << ". File does not exist." << std::endl;
			return "";
		}

		std::string line;
		while (!stream.eof())
		{
			std::getline(stream, line);
			output.append(line + "\n");
		}
		stream.close();
		return output;
	}

	struct ObjData
	{
		std::vector<ARM::Vec3> m_Vertices;
		std::vector<ARM::Vec2> m_TexCoords;
		std::vector<ARM::Vec3> m_Normals;
		std::vector<unsigned int> m_VertIndices;
		std::vector<unsigned int> m_TextIndices;
		std::vector<unsigned int> m_NormIndices;
		std::vector<ARM::Vec3> m_UniqueVertices;

		int GetSizeVertices() { return m_Vertices.size(); }
		int GetSizeTexCoords() { return m_TexCoords.size(); }
		int GetSizeNormals() { return m_Normals.size(); }
		int GetSizeVertIndices() { return m_VertIndices.size(); }
		int GetSizeTextIndices() { return m_TextIndices.size(); }
		int GetSizeNormIndices() { return m_NormIndices.size(); }
		int GetSizeUniqueVertices() { return m_UniqueVertices.size(); }
	};

	static ObjData read_obj(const char* filepath)
	{
		ObjData result;
		std::ifstream stream(filepath, std::fstream::in);

		if (!stream.is_open())
		{
			std::cout << "Could not read file " << filepath << ". File does not exist." << std::endl;
			return result;
		}
		
		
		while (!stream.eof())
		{
			std::string line;
			std::stringstream sstream;
			std::string flag;

			std::getline(stream, line);
			sstream << line;
			sstream >> flag;
			
			if (flag == "v")
			{
				float x, y, z;

				sstream >> x >> y >> z;
				result.m_Vertices.emplace_back(ARM::Vec3(x, y, z));
			}

			else if (flag == "vt")
			{
				float u, v;

				sstream >> u >> v;
				result.m_TexCoords.emplace_back(ARM::Vec2(u, v));
			}
			
			else if (flag == "vn")
			{
				float x, y, z;

				sstream >> x >> y >> z;
				result.m_Normals.emplace_back(ARM::Vec3(x, y, z));
			}

			else if (flag == "f")
			{
				std::string x, y, z;
				sstream >> x >> y >> z;

				for (int i = 0; i < 2; i++)
				{
					int x_pos = x.find('/', 0);
					x.replace(x_pos, 1, 1, ' ');

					int y_pos = y.find('/', 0);
					y.replace(y_pos, 1, 1, ' ');

					int z_pos = z.find('/', 0);
					z.replace(z_pos, 1, 1, ' ');
				}

				std::stringstream x_ss;
				x_ss << x;
				std::stringstream y_ss;
				y_ss << y;
				std::stringstream z_ss;
				z_ss << z;

				int vert_index;
				int text_index;
				int norm_index;
				unsigned int eoVertices = result.GetSizeVertices();

				x_ss >> vert_index >> text_index >> norm_index;
				if (vert_index > 0)
				{
					result.m_VertIndices.push_back(vert_index - 1);
					result.m_TextIndices.push_back(text_index - 1);
					result.m_NormIndices.push_back(norm_index - 1);
				}
				else
				{
					result.m_VertIndices.push_back(eoVertices + vert_index);
					result.m_TextIndices.push_back(eoVertices + text_index);
					result.m_NormIndices.push_back(eoVertices + norm_index);
				}

				y_ss >> vert_index >> text_index >> norm_index;
				if (vert_index > 0)
				{
					result.m_VertIndices.push_back(vert_index - 1);
					result.m_TextIndices.push_back(text_index - 1);
					result.m_NormIndices.push_back(norm_index - 1);
				}
				else
				{
					result.m_VertIndices.push_back(eoVertices + vert_index);
					result.m_TextIndices.push_back(eoVertices + text_index);
					result.m_NormIndices.push_back(eoVertices + norm_index);
				}
				
				z_ss >> vert_index >> text_index >> norm_index;
				if (vert_index > 0)
				{
					result.m_VertIndices.push_back(vert_index - 1);
					result.m_TextIndices.push_back(text_index - 1);
					result.m_NormIndices.push_back(norm_index - 1);
				}
				else
				{
					result.m_VertIndices.push_back(eoVertices + vert_index);
					result.m_TextIndices.push_back(eoVertices + text_index);
					result.m_NormIndices.push_back(eoVertices + norm_index);
				}
			}

		}
		
		result.m_UniqueVertices.reserve(result.GetSizeVertIndices());
		for (int i = 0; i < result.GetSizeVertIndices(); i++)
		{
			result.m_UniqueVertices.emplace_back(ARM::Vec3((float)result.m_VertIndices[i], (float)result.m_TextIndices[i], (float)result.m_NormIndices[i]));
		}
		
		std::sort(result.m_UniqueVertices.begin(), result.m_UniqueVertices.end(),
			[&](ARM::Vec3 a, ARM::Vec3 b) {return a.x < b.x;});
		
		for (int i = 0; i < (int)result.GetSizeVertices();)
		{
			if (i == result.GetSizeVertices() - 1) break;
			if (result.m_UniqueVertices[i].x == result.m_UniqueVertices[i + 1].x)
			{
				result.m_UniqueVertices.erase(result.m_UniqueVertices.begin() + i + 1);
			}
			if (result.m_UniqueVertices[i].x != result.m_UniqueVertices[i + 1].x)
			{
				i++;
			}
		}
		return result;
	}
};
}
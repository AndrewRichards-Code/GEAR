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
			std::cout << "ERROR: GEAR::FileUtils::read_file: Could not read file " << filepath << ". File does not exist." << std::endl;
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
			std::cout << "ERROR: GEAR::FileUtils::read_obj: Could not read file " << filepath << ". File does not exist." << std::endl;
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

	private:
	static bool IsBigEndian()
	{
		int a = 1;
		return !((char*)&a)[0];
	}

	static int ConvertToInt(char* buffer, int len)
	{
		int a = 0;
		if (!IsBigEndian())
			for (int i = 0; i<len; i++)
				((char*)&a)[i] = buffer[i];
		else
			for (int i = 0; i<len; i++)
				((char*)&a)[3 - i] = buffer[i];
		return a;
	}

	public:
	struct WavData //You must call 'delete[]' once finished, as WavData::m_Data is heap allocated with 'new'.
	{
		char* m_Data;
		int m_Channels;
		int m_SampleRate;
		int m_BitsPerSample;
		int m_Size;
	};

	static WavData load_wav(const char* filepath) //You must call 'delete[]' once finished, as WavData::m_Data is heap allocated with 'new'.
	{
		WavData result = { 0 };
		char buffer[4];
		std::ifstream stream(filepath, std::ios::binary);
		stream.read(buffer, 4);     //RIFF
		if (strncmp(buffer, "RIFF", 4) != 0)
		{
			std::cout << "ERROR: GEAR::FileUtils::read_wav: Could not read file " << filepath << ". File does not exist." << std::endl;
			return result;
		}
		stream.read(buffer, 4);
		stream.read(buffer, 4);     //WAVE
		stream.read(buffer, 4);     //fmt
		stream.read(buffer, 4);     //Subchunck1 Size
		stream.read(buffer, 2);     //Audio Format (1 = PCM)
		stream.read(buffer, 2);		//NumChannels
		result.m_Channels = ConvertToInt(buffer, 2);
		
		stream.read(buffer, 4);		//SampleRate
		result.m_SampleRate = ConvertToInt(buffer, 4);

		stream.read(buffer, 4);		//ByteRate
		stream.read(buffer, 2);		//BlockAllign
		stream.read(buffer, 2);		//BitPerSample
		result.m_BitsPerSample = ConvertToInt(buffer, 2);

		while(true)
		{
			stream.read(buffer, 4);      //data
			if (strncmp(buffer, "data ", 4) == 0)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		stream.read(buffer, 4);		 //Subchunck1 Size
		result.m_Size = ConvertToInt(buffer, 4);

		result.m_Data = new char[result.m_Size];
		stream.read(result.m_Data, result.m_Size);
		return result;
	}
};
}
#pragma once

#include "gear_core_common.h"
//#include "Assimp.h"

namespace gear 
{
namespace file_utils
{
	static std::string read_file(const std::string& filepath)
	{
		std::ifstream stream(filepath, std::fstream::in);
		std::string output;

		if (!stream.is_open())
		{
			GEAR_WARN(GEAR_ERROR_CODE::GEAR_UTILS | GEAR_ERROR_CODE::GEAR_NO_FILE, ("ERROR: GEAR::FileUtils: Could not read file " + filepath + ". File does not exist.").c_str());
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

	static std::vector<char> read_binary(const std::string& filepath)
	{
		std::ifstream stream(filepath, std::fstream::in | std::fstream::binary | std::fstream::ate);
		if (!stream.is_open())
		{
			GEAR_WARN(GEAR_ERROR_CODE::GEAR_UTILS | GEAR_ERROR_CODE::GEAR_NO_FILE, ("ERROR: GEAR::FileUtils: Could not read file " + filepath + ". File does not exist.").c_str());
			return {};
		}

		std::streamoff size = stream.tellg();
		stream.seekg(0, std::fstream::beg);
		std::vector<char> output(static_cast<unsigned int>(size));
		stream.read(output.data(), size);

		stream.close();
		return output;
	}

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

	struct WavData
	{
		const char* m_FilePath;
		std::unique_ptr<std::ifstream> m_Stream;
		std::array<char, 8192> m_Buffer1;
		std::array<char, 8192> m_Buffer2;
		int m_NextBuffer;
		int m_Channels;
		int m_SampleRate;
		int m_BitsPerSample;
		int m_Size;
		std::deque<std::streamoff> m_BufferQueue;
		bool m_LoopBufferQueue;
		
		WavData() : m_FilePath(nullptr), m_Stream(nullptr), m_Buffer1({ 0 }), m_Buffer2({ 0 }), m_NextBuffer(0), m_Channels(0), m_SampleRate(0), m_BitsPerSample(0), m_Size(0), m_BufferQueue({ 0 }), m_LoopBufferQueue(false) {};
	};

	static std::shared_ptr<WavData> stream_wav(const std::string& filepath)
	{
		std::shared_ptr<WavData> result = std::make_shared<WavData>();
		result->m_FilePath = filepath.c_str();
		result->m_NextBuffer = 1;
		result->m_LoopBufferQueue = false;

		char buffer[4];

		result->m_Stream = std::make_unique<std::ifstream>(filepath, std::ios::binary);
		result->m_Stream->read(buffer, 4);     //RIFF
		if (strncmp(buffer, "RIFF", 4) != 0)
		{
			GEAR_WARN(GEAR_ERROR_CODE::GEAR_UTILS | GEAR_ERROR_CODE::GEAR_NO_FILE, ("ERROR: GEAR::FileUtils: Could not read file " + filepath + ". File does not exist.").c_str());
			return result;
		}
		result->m_Stream->read(buffer, 4);
		result->m_Stream->read(buffer, 4);     //WAVE
		result->m_Stream->read(buffer, 4);     //fmt
		result->m_Stream->read(buffer, 4);     //Subchunck1 Size
		result->m_Stream->read(buffer, 2);     //Audio Format (1 = PCM)
		result->m_Stream->read(buffer, 2);		//NumChannels
		result->m_Channels = ConvertToInt(buffer, 2);
		
		result->m_Stream->read(buffer, 4);		//SampleRate
		result->m_SampleRate = ConvertToInt(buffer, 4);

		result->m_Stream->read(buffer, 4);		//ByteRate
		result->m_Stream->read(buffer, 2);		//BlockAllign
		result->m_Stream->read(buffer, 2);		//BitPerSample
		result->m_BitsPerSample = ConvertToInt(buffer, 2);

		while(true)
		{
			result->m_Stream->read(buffer, 4);      //data
			if (strncmp(buffer, "data ", 4) == 0)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		result->m_Stream->read(buffer, 4);		 //Subchunck2 Size
		result->m_Size = ConvertToInt(buffer, 4);

		while(result->m_Stream->tellg() <= result->m_Size)
		{
			result->m_BufferQueue.push_back(result->m_Stream->tellg());
			result->m_Stream->seekg(result->m_Buffer1.size(), std::ios_base::cur);
		}
		/*stream.seekg(0, std::ios_base::end);
		int temp = stream.tellg();*/
		return result;
	}

	static void get_next_wav_block(WavData& input)
	{
		if (!input.m_BufferQueue.empty())
		{
			input.m_Stream->seekg(input.m_BufferQueue.front(), std::ios_base::beg);
			switch (input.m_NextBuffer)
			{
			case 1:
				input.m_Stream->read(input.m_Buffer1.data(), input.m_Buffer1.size());
				input.m_NextBuffer = 2;
				break;
			case 2:
				input.m_Stream->read(input.m_Buffer2.data(), input.m_Buffer2.size());
				input.m_NextBuffer = 1;
				break;
			}

			if (input.m_LoopBufferQueue == true)
			{
				std::streamoff temp = input.m_BufferQueue.front();
				input.m_BufferQueue.pop_front();
				input.m_BufferQueue.push_back(temp);
			}
			else
			{
				input.m_BufferQueue.pop_front();
			}
			//stream.close();
		}
		else
		{
			input.m_NextBuffer = 0;
			input.m_Stream->close();
		}
	}
}
}
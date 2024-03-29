#pragma once
#include "gear_core_common.h"
#include <deque>
#include <fstream>

namespace gear 
{
	namespace utils
	{
		struct WavData
		{
			std::string						filepath;
			Scope<std::ifstream>			stream;

			std::array<char, 8192>			buffer1;
			std::array<char, 8192>			buffer2;
			uint32_t						nextBuffer;
			std::deque<std::streamoff>		bufferQueue;
			bool							loopBufferQueue;

			uint32_t						formatTag;
			uint32_t						channels;
			uint32_t						sampleRate;
			uint32_t						byteRate;
			uint32_t						blockAlign;
			uint32_t						bitsPerSample;
			uint32_t						size;


			WavData() : filepath(), stream(nullptr),
				buffer1({ 0 }), buffer2({ 0 }), nextBuffer(0), bufferQueue(), loopBufferQueue(true),
				formatTag(0), channels(0), sampleRate(0), byteRate(0), blockAlign(0), bitsPerSample(0), size(0) {};
		};

		static Ref<WavData> stream_wav(const std::string& filepath)
		{
			Ref<WavData> result = CreateRef<WavData>();
			result->filepath = filepath;
			result->nextBuffer = 1;

			result->stream = CreateScope<std::ifstream>(filepath, std::ios::binary);
			std::ifstream& stream = *(result->stream);

			auto ConvertToUint32_t = [](char* buffer, int len) -> uint32_t
			{
				uint32_t a = 0;
				uint32_t b = 1;

				bool isBigEndian = !((char*)&b)[0];

				if (!isBigEndian)
					for (int i = 0; i < len; i++)
						((char*)&a)[i] = buffer[i];
				else
					for (int i = 0; i < len; i++)
						((char*)&a)[3 - i] = buffer[i];
				return a;
			};

			char buffer[4];

			stream.read(buffer, 4);									//RIFF
			if (strncmp(buffer, "RIFF", 4) != 0)
			{
				GEAR_WARN(ErrorCode::UTILS | ErrorCode::NO_FILE, "Could not read file %s. File does not exist.", filepath);
				return result;
			}

			stream.read(buffer, 4);
			stream.read(buffer, 4);									//WAVE
			stream.read(buffer, 4);									//fmt
			stream.read(buffer, 4);									//Subchunck1 Size
			stream.read(buffer, 2);									//Audio Format (1 = PCM)
			result->formatTag = ConvertToUint32_t(buffer, 2);
			stream.read(buffer, 2);									//NumChannels
			result->channels = ConvertToUint32_t(buffer, 2);
			stream.read(buffer, 4);									//SampleRate
			result->sampleRate = ConvertToUint32_t(buffer, 4);
			stream.read(buffer, 4);									//ByteRate
			result->byteRate = ConvertToUint32_t(buffer, 4);
			stream.read(buffer, 2);									//BlockAllign
			result->blockAlign = ConvertToUint32_t(buffer, 2);
			stream.read(buffer, 2);									//BitPerSample
			result->bitsPerSample = ConvertToUint32_t(buffer, 2);

			memset(buffer, 0, 4);
			while (true)
			{
				stream.read(buffer, 1);								//data
				if (strncmp(buffer, "d", 1) == 0)
				{
					stream.read(buffer, 3);
					if (strncmp(buffer, "ata", 3) == 0)
					{
						break;
					}
				}
				else if (stream.eof())
					break;
				else
					continue;
			}

			stream.read(buffer, 4);									//Subchunck2 Size
			result->size = ConvertToUint32_t(buffer, 4);

			while (stream.tellg() <= result->size)
			{
				result->bufferQueue.push_back(stream.tellg());
				stream.seekg(result->buffer1.size(), std::ios_base::cur);
			}
			return result;
		}

		static void get_next_wav_block(Ref<WavData>& input)
		{
			if (!input->bufferQueue.empty())
			{
				input->stream->seekg(input->bufferQueue.front(), std::ios_base::beg);
				switch (input->nextBuffer)
				{
				case 1:
					input->stream->read(input->buffer1.data(), input->buffer1.size());
					input->nextBuffer = 2;
					break;
				case 2:
					input->stream->read(input->buffer2.data(), input->buffer2.size());
					input->nextBuffer = 1;
					break;
				}

				if (input->loopBufferQueue)
				{
					std::streamoff streamPosFront = input->bufferQueue.front();
					input->bufferQueue.pop_front();
					input->bufferQueue.push_back(streamPosFront);

					if (streamPosFront > input->bufferQueue.front())
					{
						input->stream->close();
						input->stream->open(input->filepath, std::ios::binary);
					}
				}
				else
				{
					input->bufferQueue.pop_front();
				}
			}
			else
			{
				input->nextBuffer = 0;
				input->stream->close();
			}
		}
	}
}
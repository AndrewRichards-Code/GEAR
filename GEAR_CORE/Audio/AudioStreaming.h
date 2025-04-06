#pragma once
#include "gear_core_common.h"
#include <deque>
#include <fstream>

namespace gear 
{
	namespace audio
	{
		struct WavData
		{
			std::filesystem::path			filepath;
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

		Ref<WavData> StreamWavData(const std::filesystem::path& filepath);
		void GetNextWavBlock(Ref<WavData>& input);
	}
}
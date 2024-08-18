#pragma once

#include <vector>

namespace gear
{
	namespace core
	{
		struct GEAR_API DataBuffer
		{
			std::vector<uint8_t> Data;

			DataBuffer()
				:Data({})
			{}

			DataBuffer(const std::vector<uint8_t>& data)
				:Data(data)
			{}

			DataBuffer(uint8_t* data, size_t size)
			{
				Data.resize(size);
				memcpy(Data.data(), data, size);
			}

			virtual ~DataBuffer()
			{
				Data.clear();
			}

			operator const std::vector<uint8_t>& () const { return Data; }
		};
	}
}
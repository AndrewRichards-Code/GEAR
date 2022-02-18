#pragma once
#include "gear_core_common.h"
#include "Transform.h"
#include "Core/Hashing.h"

namespace gear
{
namespace objects
{
	class GEAR_API ObjectInterface
	{
	public:
		struct CreateInfo
		{
			std::string	debugName;
			void* device;
		};

		virtual void Update(const Transform& transform) = 0;

		const bool& GetUpdateGPUFlag() const { return m_UpdateGPU; }
		void ResetUpdateGPUFlag() { m_UpdateGPU = false; }

	protected:
		uint64_t m_TransformHash = 0;
		uint64_t m_CreateInfoHash = 0;
		bool m_UpdateGPU = false;

		bool TransformHasChanged(const Transform& transform)
		{
			uint64_t newHash = 0;
			newHash ^= core::GetHash(transform.translation.x);
			newHash ^= core::GetHash(transform.translation.y);
			newHash ^= core::GetHash(transform.translation.z);
			newHash ^= core::GetHash(transform.orientation.s);
			newHash ^= core::GetHash(transform.orientation.i);
			newHash ^= core::GetHash(transform.orientation.j);
			newHash ^= core::GetHash(transform.orientation.k);
			newHash ^= core::GetHash(transform.scale.x);
			newHash ^= core::GetHash(transform.scale.y);
			newHash ^= core::GetHash(transform.scale.z);
			if (m_TransformHash == newHash)
			{
				m_UpdateGPU |= false;
				return false;
			}
			else
			{
				m_TransformHash = newHash;
				m_UpdateGPU |= true;
				return true;
			}
		}

		virtual bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) = 0;
		bool CompareCreateInfoHash(const uint64_t newHash)
		{
			if (m_CreateInfoHash == newHash)
			{
				m_UpdateGPU |= false;
				return false;
			}
			else
			{
				m_CreateInfoHash = newHash;
				m_UpdateGPU |= true;
				return true;
			}
		}
	};

	class GEAR_API ObjectComponentInterface
	{
	public:
		struct CreateInfo
		{
			std::string	debugName;
			void* device;
		};
		virtual void Update() = 0;

		const bool& GetUpdateGPUFlag() const { return m_UpdateGPU; }
		void ResetUpdateGPUFlag() { m_UpdateGPU = false; }

	protected:
		uint64_t m_CreateInfoHash = 0;
		bool m_UpdateGPU = false;

		virtual bool CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo) = 0;
		bool CompareCreateInfoHash(const uint64_t newHash)
		{
			if (m_CreateInfoHash == newHash)
			{
				m_UpdateGPU |= false;
				return false;
			}
			else
			{
				m_CreateInfoHash = newHash;
				m_UpdateGPU |= true;
				return true;
			}
		}
	};
}
}

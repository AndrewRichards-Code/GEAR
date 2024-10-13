#pragma once
#include "Asset/AssetMetadata.h"
#include "Asset/AssetFile.h"
#include "Asset/AssetManager.h"
#include "Project/Project.h"
#include "UI/UIContext.h"
#include "yaml-cpp/yaml.h"

namespace gear
{
	namespace asset
	{
		namespace serialiser
		{
			class GEAR_API AssetSerialiser
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);

				static Ref<asset::AssetManager> GetAssetManager() { return project::Project::GetAssetManager(); }
				static Ref<asset::EditorAssetManager> GetEditorAssetManager() { return ui::UIContext::GetUIContext()->GetEditorAssetManager(); }

				static void SetDevice(void* device);
				static void* GetDevice();

			private:
				typedef Ref<Asset>(*AssetDeserialiseFunction)(Asset::Handle, const AssetMetadata&);
				static std::map<Asset::Type, AssetDeserialiseFunction> s_AssetDeserialiseFunctions;

				typedef void(*AssetSerialiseFunctions)(Ref<Asset>, const AssetMetadata&);
				static std::map<Asset::Type, AssetSerialiseFunctions> s_AssetSerialiseFunctions;

				static inline void* s_Device = nullptr;
			};


			template<typename T>
			inline std::string ToString(const T& value)
			{
				return std::string(magic_enum::enum_name<T>(value));
			}

			template<typename T>
			inline void ToTypeEnum(T& value, const YAML::Node& node)
			{
				value = magic_enum::enum_cast<T>(std::string_view(node.as<std::string>())).value();
			}

			template<typename T>
			inline void ToType(T& value, const YAML::Node& node)
			{
				value = node.as<T>();
			}

			template<typename T>
			inline void ToTypeVector(mars::Vector2<T>& value, const YAML::Node& node)
			{
				ToType(value.x, node[0]);
				ToType(value.y, node[1]);
			}
			template<typename T>
			inline void ToTypeVector(mars::Vector3<T>& value, const YAML::Node& node)
			{
				ToType(value.x, node[0]);
				ToType(value.y, node[1]);
				ToType(value.z, node[2]);
			}
			template<typename T>
			inline void ToTypeVector(mars::Vector4<T>& value, const YAML::Node& node)
			{
				ToType(value.x, node[0]);
				ToType(value.y, node[1]);
				ToType(value.z, node[2]);
				ToType(value.w, node[3]);
			}
			inline void ToTypeQuaternion(mars::Quaternion& value, const YAML::Node& node)
			{
				ToType(value.s, node[0]);
				ToType(value.i, node[1]);
				ToType(value.j, node[2]);
				ToType(value.k, node[3]);
			}

			template<typename T>
			inline Ref<T> ToAsset(const YAML::Node& node)
			{
				uint64_t handleRaw = 0;
				ToType(handleRaw, node);
				Asset::Handle handle = Asset::Handle(handleRaw);

				const Ref<AssetManager>& assetManager = AssetSerialiser::GetAssetManager();
				if (!assetManager->IsAssetHandleValid(handle))
				{
					GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "Can not find Asset::Handle %ui in AssetRegistry.", handle.AsUint64_t());
					return nullptr;
				}

				return assetManager->GetAsset<T>(handle);
			}

			//YAML operator<<

			template<typename T>
			inline YAML::Emitter& operator<<(YAML::Emitter& data, const mars::Vector2<T>& value)
			{
				data << YAML::Flow;
				data << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
				return data;
			}
			template<typename T>
			inline YAML::Emitter& operator<<(YAML::Emitter& data, const mars::Vector3<T>& value)
			{
				data << YAML::Flow;
				data << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
				return data;
			}
			template<typename T>
			inline YAML::Emitter& operator<<(YAML::Emitter& data, const mars::Vector4<T>& value)
			{
				data << YAML::Flow;
				data << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
				return data;
			}
			inline YAML::Emitter& operator<<(YAML::Emitter& data, const mars::Quaternion& value)
			{
				data << YAML::Flow;
				data << YAML::BeginSeq << value.s << value.i << value.j << value.k << YAML::EndSeq;
				return data;
			}
		}
	}
}
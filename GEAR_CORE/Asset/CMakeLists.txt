cmake_minimum_required(VERSION 3.27)

#GEAR_CORE_Asset
set(TARGET_NAME GEAR_CORE_Asset)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Asset_SRC_HEADERS
	"Asset.h"
	"AssetDataBuffer.h"
	"AssetFile.h"
	"AssetMetadata.h"
	"AssetRegistry.h"
)
set(Asset_SRC_CPP_FILES
	"Asset.cpp"
	"AssetDataBuffer.cpp"
	"AssetFile.cpp"
	"AssetRegistry.cpp"
)

target_sources(${TARGET_NAME} PRIVATE ${Asset_SRC_CPP_FILES} ${Asset_SRC_HEADERS})

#YAML-CPP
target_link_libraries(${TARGET_NAME} PRIVATE
	yaml-cpp::yaml-cpp
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_ASSET_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)


#GEAR_CORE_AssetManager
set(TARGET_NAME GEAR_CORE_AssetManager)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

#Manager
set(AssetManager_SRC_HEADERS
	"Manager/AssetManager.h"	
	"Manager/EditorAssetManager.h"
)
set(AssetManager_SRC_CPP_FILES
	"Manager/AssetManager.cpp"
	"Manager/EditorAssetManager.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${AssetManager_SRC_CPP_FILES} ${AssetManager_SRC_HEADERS})
source_group("Manager" FILES ${AssetManager_SRC_CPP_FILES} ${AssetManager_SRC_HEADERS})

#Serialiser
set(AssetSerialiser_SRC_HEADERS
	"Serialiser/AssetSerialiser.h"
	"Serialiser/ExternalFileSerialiser.h"
	"Serialiser/MaterialSerialiser.h"
	"Serialiser/MeshSerialiser.h"
	"Serialiser/SceneSerialiser.h"
	"Serialiser/TextureSerialiser.h"
)
set(AssetSerialiser_SRC_CPP_FILES
	"Serialiser/AssetSerialiser.cpp"
	"Serialiser/ExternalFileSerialiser.cpp"
	"Serialiser/MaterialSerialiser.cpp"
	"Serialiser/MeshSerialiser.cpp"
	"Serialiser/SceneSerialiser.cpp"
	"Serialiser/TextureSerialiser.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${AssetSerialiser_SRC_CPP_FILES} ${AssetSerialiser_SRC_HEADERS})
source_group("Serialiser" FILES ${AssetSerialiser_SRC_CPP_FILES} ${AssetSerialiser_SRC_HEADERS})

#External Serialiser
set(AssetExternalSerialiser_SRC_HEADERS
	"Serialiser/ExternalSerialiser/AudioSerialiser.h"
	"Serialiser/ExternalSerialiser/FontSerialiser.h"
	"Serialiser/ExternalSerialiser/ModelLoader.h"
	"Serialiser/ExternalSerialiser/ImageSerialiser.h"
)
set(AssetExternalSerialiser_SRC_CPP_FILES
	"Serialiser/ExternalSerialiser/AudioSerialiser.cpp"
	"Serialiser/ExternalSerialiser/FontSerialiser.cpp"
	"Serialiser/ExternalSerialiser/ModelLoader.cpp"
	"Serialiser/ExternalSerialiser/ImageSerialiser.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${AssetExternalSerialiser_SRC_CPP_FILES} ${AssetExternalSerialiser_SRC_HEADERS})
source_group("Serialiser\\External Serialiser" FILES ${AssetExternalSerialiser_SRC_CPP_FILES} ${AssetExternalSerialiser_SRC_HEADERS})

#ASSIMP
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/ASSIMP/include/"
)
target_link_libraries(${TARGET_NAME} PRIVATE assimp)

#YAML-CPP
target_link_libraries(${TARGET_NAME} PRIVATE
	yaml-cpp::yaml-cpp
)

#FREETYPE
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/FREETYPE/include/"
)
target_link_directories(${TARGET_NAME} PRIVATE
	"${GEAR_SOURCE_DIR}/External/FREETYPE/release dll/win64"
)
target_link_libraries(${TARGET_NAME} PRIVATE
	freetype.lib
)
CopyToBinaryDirPostBuild(${TARGET_NAME} "${GEAR_SOURCE_DIR}/External/FREETYPE/release dll/win64/freetype.dll" "")

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_ASSET_MANAGER_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE
	GEAR_CORE_Scene
	GEAR_CORE_Objects
	GEAR_CORE_Graphics
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)
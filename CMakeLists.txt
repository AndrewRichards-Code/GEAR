cmake_minimum_required(VERSION 3.27)

project(GEAR)

message(STATUS "GEAR")

include("External/MIRU/External/ARC/src/CMakeFunctions.cmake")
include(FetchContent)

#External Downloads
#ENTT
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/ENTT/single_include/entt/entt.hpp")
	file(DOWNLOAD 
		https://raw.githubusercontent.com/skypjack/entt/master/single_include/entt/entt.hpp
		"${CMAKE_CURRENT_SOURCE_DIR}/External/ENTT/single_include/entt/entt.hpp"
	)
	message(STATUS "Downloaded ENTT.")
else()
	message(STATUS "Found ENTT.")
endif()

#FREETYPE
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/include/ft2build.h")
	set(GEAR_FREETYPE_VERSION 2.13.2)
	set(GEAR_FREETYPE_ARCHIVE_NAME freetype-windows-binaries-${GEAR_FREETYPE_VERSION})
	set(GEAR_FREETYPE_ARCHIVE ${GEAR_FREETYPE_ARCHIVE_NAME}.zip)
	file(DOWNLOAD
		https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v${GEAR_FREETYPE_VERSION}.zip
		EXPECTED_HASH MD5=d7b15cff717e6d032c856e40e763fce2
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/${GEAR_FREETYPE_ARCHIVE}"
	)
	file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/${GEAR_FREETYPE_ARCHIVE}"
		DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/"
	)
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/${GEAR_FREETYPE_ARCHIVE_NAME}/"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/"
	)
	file(REMOVE_RECURSE 
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/${GEAR_FREETYPE_ARCHIVE}"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/${GEAR_FREETYPE_ARCHIVE_NAME}"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/demos"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/CHANGES.txt"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/FREETYPE/README.md"
	)
	message(STATUS "Downloaded FREETYPE.")
else()
	message(STATUS "Found FREETYPE.")
endif()

#GLFW
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/include/GLFW/glfw3.h")
	set(GEAR_GLFW_VERSION 3.3.8)
	set(GEAR_GLFW_ARCHIVE_NAME glfw-${GEAR_GLFW_VERSION}.bin.WIN64)
	set(GEAR_GLFW_ARCHIVE ${GEAR_GLFW_ARCHIVE_NAME}.zip)
	file(DOWNLOAD 
		https://github.com/glfw/glfw/releases/download/${GEAR_GLFW_VERSION}/${GEAR_GLFW_ARCHIVE}
		EXPECTED_HASH MD5=00ae95c16b81ae736c98c2c102409263
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/${GEAR_GLFW_ARCHIVE}"
	)
	file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/${GEAR_GLFW_ARCHIVE}"
		DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/"
	)
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/${GEAR_GLFW_ARCHIVE_NAME}/"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/"
	)
	file(REMOVE_RECURSE 
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/${GEAR_GLFW_ARCHIVE}"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/${GEAR_GLFW_ARCHIVE_NAME}"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/docs"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-mingw-w64"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-static-ucrt"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-vc2012"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-vc2013"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-vc2015"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-vc2017"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/lib-vc2019"
		"${CMAKE_CURRENT_SOURCE_DIR}/External/GLFW/README.md"
	)
	message(STATUS "Downloaded GLFW.")
else()
	message(STATUS "Found GLFW.")
endif()

#YAML-CPP
set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_BUILD_TESTS OFF)
set(YAML_CPP_INSTALL OFF)
set(YAML_CPP_BUILD_CONTRIB OFF)
set(YAML_BUILD_SHARED_LIBS OFF)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_TAG master
	SOURCE_DIR yaml-cpp
)
FetchContent_MakeAvailable(yaml-cpp)
message(STATUS "Fetch Content YAML-CPP.")
set_target_properties(yaml-cpp PROPERTIES FOLDER "External/YAML-CPP")

#Project Folders
message(STATUS "Adding ASSIMP.")
set(ASSIMP_INSTALL OFF)
set(ASSIMP_INSTALL_PDB OFF)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(External/ASSIMP)
set(CMAKE_DEBUG_POSTFIX "")
message(STATUS "Finished ASSIMP.")

set_target_properties(assimp PROPERTIES FOLDER "External/ASSIMP")
set_target_properties(zlibstatic PROPERTIES FOLDER "External/ASSIMP")
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "External/ASSIMP")

set(MIRU_BUILD_MIRU_TEST OFF)
add_subdirectory(External/MIRU)

set_target_properties(ARC PROPERTIES FOLDER "External/MIRU/External")
set_target_properties(MIRU_CORE PROPERTIES FOLDER "External/MIRU")
set_target_properties(MIRU_SHADER_COMPILER PROPERTIES FOLDER "External/MIRU")

add_subdirectory(GEAR_CORE)
add_subdirectory(GEAR_NATIVE_SCRIPT)
#add_subdirectory(GEAR_TEST)
add_subdirectory(GEARBOX)

set_property(DIRECTORY "${CMAKE_SOURCE_DIR}/GEARBOX" PROPERTY VS_STARTUP_PROJECT GEARBOX)

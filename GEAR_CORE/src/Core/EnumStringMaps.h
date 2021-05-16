#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	static std::map<std::string, miru::crossplatform::Shader::StageBit> ShaderStageBitStrings =
	{
		{ "VERTEX_BIT", miru::crossplatform::Shader::StageBit::VERTEX_BIT },
		{ "TESSELLATION_CONTROL_BIT", miru::crossplatform::Shader::StageBit::TESSELLATION_CONTROL_BIT },
		{ "TESSELLATION_EVALUATION_BIT", miru::crossplatform::Shader::StageBit::TESSELLATION_EVALUATION_BIT },
		{ "GEOMETRY_BIT", miru::crossplatform::Shader::StageBit::GEOMETRY_BIT },
		{ "FRAGMENT_BIT", miru::crossplatform::Shader::StageBit::FRAGMENT_BIT },
		{ "COMPUTE_BIT", miru::crossplatform::Shader::StageBit::COMPUTE_BIT },
		{ "ALL_GRAPHICS", miru::crossplatform::Shader::StageBit::ALL_GRAPHICS },
		{ "PIXEL_BIT", miru::crossplatform::Shader::StageBit::PIXEL_BIT },
		{ "HULL_BIT", miru::crossplatform::Shader::StageBit::HULL_BIT },
		{ "DOMAIN_BIT", miru::crossplatform::Shader::StageBit::DOMAIN_BIT }
	};

	static std::map<std::string, miru::crossplatform::PrimitiveTopology> PrimitiveTopologyStrings =
	{
		{ "POINT_LIST", miru::crossplatform::PrimitiveTopology::POINT_LIST },
		{ "LINE_LIST", miru::crossplatform::PrimitiveTopology::LINE_LIST },
		{ "LINE_STRIP", miru::crossplatform::PrimitiveTopology::LINE_STRIP },
		{ "TRIANGLE_LIST", miru::crossplatform::PrimitiveTopology::TRIANGLE_LIST },
		{ "TRIANGLE_STRIP", miru::crossplatform::PrimitiveTopology::TRIANGLE_STRIP },
		{ "TRIANGLE_FAN", miru::crossplatform::PrimitiveTopology::TRIANGLE_FAN },
		{ "LINE_LIST_WITH_ADJACENCY", miru::crossplatform::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY },
		{ "LINE_STRIP_WITH_ADJACENCY", miru::crossplatform::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY },
		{ "TRIANGLE_LIST_WITH_ADJACENCY", miru::crossplatform::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY },
		{ "TRIANGLE_STRIP_WITH_ADJACENCY", miru::crossplatform::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY },
		{ "PATCH_LIST", miru::crossplatform::PrimitiveTopology::PATCH_LIST }
	};
	
	static std::map<std::string, miru::crossplatform::PolygonMode> PolygonModeStrings =
	{
		{ "FILL", miru::crossplatform::PolygonMode::FILL },
		{ "LINE", miru::crossplatform::PolygonMode::LINE },
		{ "POINT", miru::crossplatform::PolygonMode::POINT }
	};
	
	static std::map<std::string, miru::crossplatform::CullModeBit> CullModeBitStrings =
	{
		{ "NONE_BIT", miru::crossplatform::CullModeBit::NONE_BIT },
		{ "FRONT_BIT", miru::crossplatform::CullModeBit::FRONT_BIT },
		{ "BACK_BIT", miru::crossplatform::CullModeBit::BACK_BIT },
		{ "FRONT_AND_BACK_BIT", miru::crossplatform::CullModeBit::FRONT_AND_BACK_BIT }
	};
	
	static std::map<std::string, miru::crossplatform::FrontFace> FrontFaceStrings =
	{
		{ "COUNTER_CLOCKWISE", miru::crossplatform::FrontFace::COUNTER_CLOCKWISE },
		{ "CLOCKWISE", miru::crossplatform::FrontFace::CLOCKWISE }
	};
	
	static std::map<std::string, miru::crossplatform::Image::SampleCountBit> SampleCountBitStrings
	{
		{ "SAMPLE_COUNT_1_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT },
		{ "SAMPLE_COUNT_2_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_2_BIT },
		{ "SAMPLE_COUNT_4_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_4_BIT },
		{ "SAMPLE_COUNT_8_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_8_BIT },
		{ "SAMPLE_COUNT_16_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_16_BIT },
		{ "SAMPLE_COUNT_32_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_32_BIT },
		{ "SAMPLE_COUNT_64_BIT", miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_64_BIT }
	};
	
	static std::map<std::string, miru::crossplatform::StencilOp> StencilOpStrings =
	{
		{ "KEEP", miru::crossplatform::StencilOp::KEEP },
		{ "ZERO", miru::crossplatform::StencilOp::ZERO },
		{ "REPLACE", miru::crossplatform::StencilOp::REPLACE },
		{ "INCREMENT_AND_CLAMP", miru::crossplatform::StencilOp::INCREMENT_AND_CLAMP },
		{ "DECREMENT_AND_CLAMP", miru::crossplatform::StencilOp::DECREMENT_AND_CLAMP },
		{ "INVERT", miru::crossplatform::StencilOp::INVERT },
		{ "INCREMENT_AND_WRAP", miru::crossplatform::StencilOp::INCREMENT_AND_WRAP },
		{ "DECREMENT_AND_WRAP", miru::crossplatform::StencilOp::DECREMENT_AND_WRAP }
	};
	
	static std::map<std::string, miru::crossplatform::CompareOp> CompareOpStrings =
	{
		{ "NEVER", miru::crossplatform::CompareOp::NEVER },
		{ "LESS", miru::crossplatform::CompareOp::LESS },
		{ "EQUAL", miru::crossplatform::CompareOp::EQUAL },
		{ "LESS_OR_EQUAL", miru::crossplatform::CompareOp::LESS_OR_EQUAL },
		{ "GREATER", miru::crossplatform::CompareOp::GREATER },
		{ "NOT_EQUAL", miru::crossplatform::CompareOp::NOT_EQUAL },
		{ "GREATER_OR_EQUAL", miru::crossplatform::CompareOp::GREATER_OR_EQUAL },
		{ "ALWAYS", miru::crossplatform::CompareOp::ALWAYS }
	};
	
	static std::map<std::string, miru::crossplatform::BlendFactor> BlendFactorStrings =
	{
		{ "ZERO", miru::crossplatform::BlendFactor::ZERO },
		{ "ONE", miru::crossplatform::BlendFactor::ONE },
		{ "SRC_COLOUR", miru::crossplatform::BlendFactor::SRC_COLOUR },
		{ "ONE_MINUS_SRC_COLOUR", miru::crossplatform::BlendFactor::ONE_MINUS_SRC_COLOUR },
		{ "DST_COLOUR", miru::crossplatform::BlendFactor::DST_COLOUR },
		{ "ONE_MINUS_DST_COLOUR", miru::crossplatform::BlendFactor::ONE_MINUS_DST_COLOUR },
		{ "SRC_ALPHA", miru::crossplatform::BlendFactor::SRC_ALPHA },
		{ "ONE_MINUS_SRC_ALPHA", miru::crossplatform::BlendFactor::ONE_MINUS_SRC_ALPHA },
		{ "DST_ALPHA", miru::crossplatform::BlendFactor::DST_ALPHA },
		{ "ONE_MINUS_DST_ALPHA", miru::crossplatform::BlendFactor::ONE_MINUS_DST_ALPHA },
		{ "CONSTANT_COLOUR", miru::crossplatform::BlendFactor::CONSTANT_COLOUR },
		{ "ONE_MINUS_CONSTANT_COLOUR", miru::crossplatform::BlendFactor::ONE_MINUS_CONSTANT_COLOUR },
		{ "CONSTANT_ALPHA", miru::crossplatform::BlendFactor::CONSTANT_ALPHA },
		{ "ONE_MINUS_CONSTANT_ALPHA", miru::crossplatform::BlendFactor::ONE_MINUS_CONSTANT_ALPHA },
		{ "SRC_ALPHA_SATURATE", miru::crossplatform::BlendFactor::SRC_ALPHA_SATURATE },
		{ "SRC1_COLOUR", miru::crossplatform::BlendFactor::SRC1_COLOUR },
		{ "ONE_MINUS_SRC1_COLOUR", miru::crossplatform::BlendFactor::ONE_MINUS_SRC1_COLOUR },
		{ "SRC1_ALPHA", miru::crossplatform::BlendFactor::SRC1_ALPHA },
		{ "ONE_MINUS_SRC1_ALPHA }", miru::crossplatform::BlendFactor::ONE_MINUS_SRC1_ALPHA }
	};
	
	static std::map<std::string, miru::crossplatform::BlendOp> BlendOpStrings =
	{
		{ "ADD", miru::crossplatform::BlendOp::ADD },
		{ "SUBTRACT", miru::crossplatform::BlendOp::SUBTRACT },
		{ "REVERSE_SUBTRACT", miru::crossplatform::BlendOp::REVERSE_SUBTRACT },
		{ "MIN", miru::crossplatform::BlendOp::MIN },
		{ "MAX", miru::crossplatform::BlendOp::MAX }
	};
	
	static std::map<std::string, miru::crossplatform::ColourComponentBit> ColourComponentBitStrings =
	{
		{ "R_BIT", miru::crossplatform::ColourComponentBit::R_BIT },
		{ "G_BIT", miru::crossplatform::ColourComponentBit::G_BIT },
		{ "B_BIT", miru::crossplatform::ColourComponentBit::B_BIT },
		{ "A_BIT", miru::crossplatform::ColourComponentBit::A_BIT },
	};
	
	static std::map<std::string, miru::crossplatform::LogicOp> LogicOpStrings
	{
		{"CLEAR", miru::crossplatform::LogicOp::CLEAR },
		{"AND", miru::crossplatform::LogicOp::AND },
		{"AND_REVERSE", miru::crossplatform::LogicOp::AND_REVERSE },
		{"COPY", miru::crossplatform::LogicOp::COPY },
		{"AND_INVERTED", miru::crossplatform::LogicOp::AND_INVERTED },
		{"NO_OP", miru::crossplatform::LogicOp::NO_OP },
		{"XOR", miru::crossplatform::LogicOp::XOR },
		{"OR", miru::crossplatform::LogicOp::OR },
		{"NOR", miru::crossplatform::LogicOp::NOR },
		{"EQUIVALENT", miru::crossplatform::LogicOp::EQUIVALENT },
		{"INVERT", miru::crossplatform::LogicOp::INVERT },
		{"OR_REVERSE", miru::crossplatform::LogicOp::OR_REVERSE },
		{"COPY_INVERTED", miru::crossplatform::LogicOp::COPY_INVERTED },
		{"OR_INVERTED", miru::crossplatform::LogicOp::OR_INVERTED },
		{"NAND", miru::crossplatform::LogicOp::NAND },
		{"SET ", miru::crossplatform::LogicOp::SET }
	};
}
}
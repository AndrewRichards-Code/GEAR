#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace core
	{
		static std::map<std::string, miru::base::Shader::StageBit> ShaderStageBitStrings =
		{
			{ "VERTEX_BIT", miru::base::Shader::StageBit::VERTEX_BIT },
			{ "TESSELLATION_CONTROL_BIT", miru::base::Shader::StageBit::TESSELLATION_CONTROL_BIT },
			{ "TESSELLATION_EVALUATION_BIT", miru::base::Shader::StageBit::TESSELLATION_EVALUATION_BIT },
			{ "GEOMETRY_BIT", miru::base::Shader::StageBit::GEOMETRY_BIT },
			{ "FRAGMENT_BIT", miru::base::Shader::StageBit::FRAGMENT_BIT },
			{ "COMPUTE_BIT", miru::base::Shader::StageBit::COMPUTE_BIT },
			{ "ALL_GRAPHICS", miru::base::Shader::StageBit::ALL_GRAPHICS },
			{ "PIXEL_BIT", miru::base::Shader::StageBit::PIXEL_BIT },
			{ "HULL_BIT", miru::base::Shader::StageBit::HULL_BIT },
			{ "DOMAIN_BIT", miru::base::Shader::StageBit::DOMAIN_BIT }
		};

		static std::map<std::string, miru::base::PrimitiveTopology> PrimitiveTopologyStrings =
		{
			{ "POINT_LIST", miru::base::PrimitiveTopology::POINT_LIST },
			{ "LINE_LIST", miru::base::PrimitiveTopology::LINE_LIST },
			{ "LINE_STRIP", miru::base::PrimitiveTopology::LINE_STRIP },
			{ "TRIANGLE_LIST", miru::base::PrimitiveTopology::TRIANGLE_LIST },
			{ "TRIANGLE_STRIP", miru::base::PrimitiveTopology::TRIANGLE_STRIP },
			{ "TRIANGLE_FAN", miru::base::PrimitiveTopology::TRIANGLE_FAN },
			{ "LINE_LIST_WITH_ADJACENCY", miru::base::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY },
			{ "LINE_STRIP_WITH_ADJACENCY", miru::base::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY },
			{ "TRIANGLE_LIST_WITH_ADJACENCY", miru::base::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY },
			{ "TRIANGLE_STRIP_WITH_ADJACENCY", miru::base::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY },
			{ "PATCH_LIST", miru::base::PrimitiveTopology::PATCH_LIST }
		};

		static std::map<std::string, miru::base::PolygonMode> PolygonModeStrings =
		{
			{ "FILL", miru::base::PolygonMode::FILL },
			{ "LINE", miru::base::PolygonMode::LINE },
			{ "POINT", miru::base::PolygonMode::POINT }
		};

		static std::map<std::string, miru::base::CullModeBit> CullModeBitStrings =
		{
			{ "NONE_BIT", miru::base::CullModeBit::NONE_BIT },
			{ "FRONT_BIT", miru::base::CullModeBit::FRONT_BIT },
			{ "BACK_BIT", miru::base::CullModeBit::BACK_BIT },
			{ "FRONT_AND_BACK_BIT", miru::base::CullModeBit::FRONT_AND_BACK_BIT }
		};

		static std::map<std::string, miru::base::FrontFace> FrontFaceStrings =
		{
			{ "COUNTER_CLOCKWISE", miru::base::FrontFace::COUNTER_CLOCKWISE },
			{ "CLOCKWISE", miru::base::FrontFace::CLOCKWISE }
		};

		static std::map<std::string, miru::base::Image::SampleCountBit> SampleCountBitStrings
		{
			{ "SAMPLE_COUNT_1_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT },
			{ "SAMPLE_COUNT_2_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_2_BIT },
			{ "SAMPLE_COUNT_4_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_4_BIT },
			{ "SAMPLE_COUNT_8_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_8_BIT },
			{ "SAMPLE_COUNT_16_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_16_BIT },
			{ "SAMPLE_COUNT_32_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_32_BIT },
			{ "SAMPLE_COUNT_64_BIT", miru::base::Image::SampleCountBit::SAMPLE_COUNT_64_BIT }
		};

		static std::map<std::string, miru::base::StencilOp> StencilOpStrings =
		{
			{ "KEEP", miru::base::StencilOp::KEEP },
			{ "ZERO", miru::base::StencilOp::ZERO },
			{ "REPLACE", miru::base::StencilOp::REPLACE },
			{ "INCREMENT_AND_CLAMP", miru::base::StencilOp::INCREMENT_AND_CLAMP },
			{ "DECREMENT_AND_CLAMP", miru::base::StencilOp::DECREMENT_AND_CLAMP },
			{ "INVERT", miru::base::StencilOp::INVERT },
			{ "INCREMENT_AND_WRAP", miru::base::StencilOp::INCREMENT_AND_WRAP },
			{ "DECREMENT_AND_WRAP", miru::base::StencilOp::DECREMENT_AND_WRAP }
		};

		static std::map<std::string, miru::base::CompareOp> CompareOpStrings =
		{
			{ "NEVER", miru::base::CompareOp::NEVER },
			{ "LESS", miru::base::CompareOp::LESS },
			{ "EQUAL", miru::base::CompareOp::EQUAL },
			{ "LESS_OR_EQUAL", miru::base::CompareOp::LESS_OR_EQUAL },
			{ "GREATER", miru::base::CompareOp::GREATER },
			{ "NOT_EQUAL", miru::base::CompareOp::NOT_EQUAL },
			{ "GREATER_OR_EQUAL", miru::base::CompareOp::GREATER_OR_EQUAL },
			{ "ALWAYS", miru::base::CompareOp::ALWAYS }
		};

		static std::map<std::string, miru::base::BlendFactor> BlendFactorStrings =
		{
			{ "ZERO", miru::base::BlendFactor::ZERO },
			{ "ONE", miru::base::BlendFactor::ONE },
			{ "SRC_COLOUR", miru::base::BlendFactor::SRC_COLOUR },
			{ "ONE_MINUS_SRC_COLOUR", miru::base::BlendFactor::ONE_MINUS_SRC_COLOUR },
			{ "DST_COLOUR", miru::base::BlendFactor::DST_COLOUR },
			{ "ONE_MINUS_DST_COLOUR", miru::base::BlendFactor::ONE_MINUS_DST_COLOUR },
			{ "SRC_ALPHA", miru::base::BlendFactor::SRC_ALPHA },
			{ "ONE_MINUS_SRC_ALPHA", miru::base::BlendFactor::ONE_MINUS_SRC_ALPHA },
			{ "DST_ALPHA", miru::base::BlendFactor::DST_ALPHA },
			{ "ONE_MINUS_DST_ALPHA", miru::base::BlendFactor::ONE_MINUS_DST_ALPHA },
			{ "CONSTANT_COLOUR", miru::base::BlendFactor::CONSTANT_COLOUR },
			{ "ONE_MINUS_CONSTANT_COLOUR", miru::base::BlendFactor::ONE_MINUS_CONSTANT_COLOUR },
			{ "CONSTANT_ALPHA", miru::base::BlendFactor::CONSTANT_ALPHA },
			{ "ONE_MINUS_CONSTANT_ALPHA", miru::base::BlendFactor::ONE_MINUS_CONSTANT_ALPHA },
			{ "SRC_ALPHA_SATURATE", miru::base::BlendFactor::SRC_ALPHA_SATURATE },
			{ "SRC1_COLOUR", miru::base::BlendFactor::SRC1_COLOUR },
			{ "ONE_MINUS_SRC1_COLOUR", miru::base::BlendFactor::ONE_MINUS_SRC1_COLOUR },
			{ "SRC1_ALPHA", miru::base::BlendFactor::SRC1_ALPHA },
			{ "ONE_MINUS_SRC1_ALPHA }", miru::base::BlendFactor::ONE_MINUS_SRC1_ALPHA }
		};

		static std::map<std::string, miru::base::BlendOp> BlendOpStrings =
		{
			{ "ADD", miru::base::BlendOp::ADD },
			{ "SUBTRACT", miru::base::BlendOp::SUBTRACT },
			{ "REVERSE_SUBTRACT", miru::base::BlendOp::REVERSE_SUBTRACT },
			{ "MIN", miru::base::BlendOp::MIN },
			{ "MAX", miru::base::BlendOp::MAX }
		};

		static std::map<std::string, miru::base::ColourComponentBit> ColourComponentBitStrings =
		{
			{ "R_BIT", miru::base::ColourComponentBit::R_BIT },
			{ "G_BIT", miru::base::ColourComponentBit::G_BIT },
			{ "B_BIT", miru::base::ColourComponentBit::B_BIT },
			{ "A_BIT", miru::base::ColourComponentBit::A_BIT },
		};

		static std::map<std::string, miru::base::LogicOp> LogicOpStrings
		{
			{"CLEAR", miru::base::LogicOp::CLEAR },
			{"AND", miru::base::LogicOp::AND },
			{"AND_REVERSE", miru::base::LogicOp::AND_REVERSE },
			{"COPY", miru::base::LogicOp::COPY },
			{"AND_INVERTED", miru::base::LogicOp::AND_INVERTED },
			{"NO_OP", miru::base::LogicOp::NO_OP },
			{"XOR", miru::base::LogicOp::XOR },
			{"OR", miru::base::LogicOp::OR },
			{"NOR", miru::base::LogicOp::NOR },
			{"EQUIVALENT", miru::base::LogicOp::EQUIVALENT },
			{"INVERT", miru::base::LogicOp::INVERT },
			{"OR_REVERSE", miru::base::LogicOp::OR_REVERSE },
			{"COPY_INVERTED", miru::base::LogicOp::COPY_INVERTED },
			{"OR_INVERTED", miru::base::LogicOp::OR_INVERTED },
			{"NAND", miru::base::LogicOp::NAND },
			{"SET ", miru::base::LogicOp::SET }
		};
	}
}
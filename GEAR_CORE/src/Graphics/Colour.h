#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		enum class ColourSpace : uint32_t
		{
			CIE_XYZ,
			SRGB,
			REC_709,
			REC_2020,
		};
		enum class GammaSpace : uint32_t
		{
			LINEAR,
			SRGB,
			REC_709,
			REC_2020,
		};

		//Forward Declarations
		class Colour_CIE_XYZ;
		class Colour_sRGB;
		class Colour_Rec709;
		class Colour_Rec2020;

		//xyz FP32 in CIE xyY Colour Space : Linear Gamma Space 
		//x = x
		//y = y 
		//z = Y
		typedef mars::float3 Colour_CIE_xyY;

		//Base Colour implementation
		class GEAR_API Colour_Base
		{
		protected:
			ColourSpace m_ColourSpace;
			GammaSpace m_GammaSpace;

			Colour_CIE_xyY m_PrimaryR;
			Colour_CIE_xyY m_PrimaryG;
			Colour_CIE_xyY m_PrimaryB;

			static Colour_CIE_xyY D65;

			mars::float3x3 RGB_To_XYZ;
			mars::float3x3 XYZ_To_RGB;

		protected:
			virtual ~Colour_Base() = default;
			virtual void Initialise() = 0;
			void ConstructTransformationMatrices();

		public:
			static Colour_CIE_xyY XYZ_To_xyY(const Colour_CIE_XYZ& in);
			static Colour_CIE_XYZ xyY_To_XYZ(const Colour_CIE_xyY& in);
		};

		//XYZ FP32 : CIE XYZ Colour Space : Linear Gamma Space
		class GEAR_API Colour_CIE_XYZ final : public Colour_Base
		{
		public:
			float x;
			float y;
			float z;

		public:
			virtual ~Colour_CIE_XYZ() = default;

			Colour_CIE_XYZ();
			Colour_CIE_XYZ(float X, float Y, float Z);
			Colour_CIE_XYZ(const mars::float3& colour);

			operator mars::float3() const
			{
				return { x, y, z };
			}

			Colour_sRGB ToSRGB();
			Colour_Rec709 ToRec709();
			Colour_Rec2020 ToRec2020();

			void Initialise() override;
		};

		//RGBA UINT8 : sRGB Colour Space : sRGB Gamma Space
		class GEAR_API Colour_sRGB final : public Colour_Base
		{
		public:
			//RGBA UINT8 : sRGB Colour Space : Linear Gamma Space
			typedef mars::float4 Colour_Linear_sRGB;

		public:
			uint8_t r : 8;
			uint8_t g : 8;
			uint8_t b : 8;
			uint8_t a : 8;

		private:
			static const float s_ToLinearSRGB_LUT[256];
			static const float s_OneOver255;

		public:
			virtual ~Colour_sRGB() = default;

			Colour_sRGB();
			Colour_sRGB(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255);
			Colour_sRGB(const mars::float4& RGBA);

			Colour_CIE_XYZ ToCIE_XYZ();
			Colour_sRGB FromCIE_XYZ(const Colour_CIE_XYZ& XYZ);

			float GetLuminance();

			Colour_Linear_sRGB Linearise();
			Colour_Linear_sRGB Linearise_LUT();
			Colour_sRGB GammaCorrection(const Colour_Linear_sRGB& lsRGB);

		private:
			void Initialise() override;

			//Normalised to 0.0f to 1.0f
			operator mars::float3() const
			{
				return mars::float3(
					std::clamp(static_cast<float>(r) / 255.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(g) / 255.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(b) / 255.0f, 0.0f, 1.0f));
			}
		};

		//RGBA UINT8 : Rec 709 Colour Space : Rec 709 Gamma Space
		class GEAR_API Colour_Rec709 final : public Colour_Base
		{
		public:
			//RGBA UINT8 : Rec 709 Colour Space : Linear Gamma Space
			typedef mars::float4 Colour_Linear_Rec709;

		public:
			uint8_t r : 8;
			uint8_t g : 8;
			uint8_t b : 8;
			uint8_t a : 8;

		private:
			//static const float s_ToLinearSRGB_LUT[256];
			static const float s_OneOver255;

		public:
			virtual ~Colour_Rec709() = default;

			Colour_Rec709();
			Colour_Rec709(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255);
			Colour_Rec709(const mars::float4& RGBA);

			Colour_CIE_XYZ ToCIE_XYZ();
			Colour_Rec709 FromCIE_XYZ(const Colour_CIE_XYZ& XYZ);

			float GetLuminance();

			Colour_Linear_Rec709 Linearise();
			Colour_Linear_Rec709 Linearise_LUT();
			Colour_Rec709 GammaCorrection(const Colour_Linear_Rec709& lRec709);

		private:
			void Initialise() override;

			//Normalised to 0.0f to 1.0f
			operator mars::float3() const
			{
				return mars::float3(
					std::clamp(static_cast<float>(r) / 255.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(g) / 255.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(b) / 255.0f, 0.0f, 1.0f));
			}
		};

		//RGBA UINT10:10:10:2 : Rec 2020 Colour Space : Rec 2020 Gamma Space
		class GEAR_API Colour_Rec2020 final : public Colour_Base
		{
		public:
			//RGBA UINT10:10:10:2 : Rec 2020 Colour Space : Linear Gamma Space
			typedef mars::float4 Colour_Linear_Rec2020;

		public:
			uint32_t r : 10;
			uint32_t g : 10;
			uint32_t b : 10;
			uint32_t a : 2;

		private:
			//static const float s_ToLinearSRGB_LUT[256];
			static const float s_OneOver3;

		public:
			virtual ~Colour_Rec2020() = default;

			Colour_Rec2020();
			Colour_Rec2020(uint32_t R, uint32_t G, uint32_t B, uint32_t A = 3);
			Colour_Rec2020(const mars::float4& RGBA);

			Colour_CIE_XYZ ToCIE_XYZ();
			Colour_Rec2020 FromCIE_XYZ(const Colour_CIE_XYZ& XYZ);

			float GetLuminance();

			Colour_Linear_Rec2020 Linearise();
			Colour_Linear_Rec2020 Linearise_LUT();
			Colour_Rec2020 GammaCorrection(const Colour_Linear_Rec2020& lRec2020);

		private:
			void Initialise() override;

			//Normalised to 0.0f to 1.0f
			operator mars::float3() const
			{
				return mars::float3(
					std::clamp(static_cast<float>(r) / 1023.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(g) / 1023.0f, 0.0f, 1.0f),
					std::clamp(static_cast<float>(b) / 1023.0f, 0.0f, 1.0f));
			}
		};
	}
}
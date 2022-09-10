#pragma once

#include "../Core/TypesCppHlsl.h"

#ifdef __cplusplus
#define CONST_REF const&
#else
#define CONST_REF
#endif

#ifdef __cplusplus
namespace gear
{
	namespace colour
	{
#endif
		static GEAR_FLOAT3 Colour_Base_D65 = GEAR_FLOAT3(0.3127f, 0.3290f, 1.0000f);

		static GEAR_FLOAT3 Colour_CIE_XYZ_PrimaryR = GEAR_FLOAT3(1.0f, 0.0f, 0.0f);
		static GEAR_FLOAT3 Colour_CIE_XYZ_PrimaryG = GEAR_FLOAT3(0.0f, 1.0f, 1.0f);
		static GEAR_FLOAT3 Colour_CIE_XYZ_PrimaryB = GEAR_FLOAT3(0.0f, 0.0f, 0.0f);

		static GEAR_FLOAT3 Colour_sRGB_PrimaryR = GEAR_FLOAT3(0.6400f, 0.3300f, 0.2126f);
		static GEAR_FLOAT3 Colour_sRGB_PrimaryG = GEAR_FLOAT3(0.3000f, 0.6000f, 0.7152f);
		static GEAR_FLOAT3 Colour_sRGB_PrimaryB = GEAR_FLOAT3(0.1500f, 0.0600f, 0.0722f);

		static GEAR_FLOAT3 Colour_Rec709_PrimaryR = GEAR_FLOAT3(0.6400f, 0.3300f, 0.2126f);
		static GEAR_FLOAT3 Colour_Rec709_PrimaryG = GEAR_FLOAT3(0.3000f, 0.6000f, 0.7152f);
		static GEAR_FLOAT3 Colour_Rec709_PrimaryB = GEAR_FLOAT3(0.1500f, 0.0600f, 0.0722f);

		static GEAR_FLOAT3 Colour_Rec2020_PrimaryR = GEAR_FLOAT3(0.7080f, 0.2920f, 0.2627f);
		static GEAR_FLOAT3 Colour_Rec2020_PrimaryG = GEAR_FLOAT3(0.1700f, 0.7970f, 0.6780f);
		static GEAR_FLOAT3 Colour_Rec2020_PrimaryB = GEAR_FLOAT3(0.1310f, 0.0460f, 0.0593f);

		GEAR_FLOAT3 Colour_Base_XYZ_To_xyY(GEAR_FLOAT3 CONST_REF value)
		{
			const float X = value.x;
			const float Y = value.y;
			const float Z = value.z;

			float denom = X + Y + Z;
			if (denom > 0.0f)
			{
				float x = X / denom;
				float y = Y / denom;
				return GEAR_FLOAT3(x, y, Y);
			}
			else
			{
				return GEAR_FLOAT3(Colour_Base_D65.x, Colour_Base_D65.y, Y);
			}
		}
		GEAR_FLOAT3 Colour_Base_xyY_To_XYZ(GEAR_FLOAT3 CONST_REF value)
		{
			const float x = value.x;
			const float y = value.y;
			const float Y = value.z;

			if (x > 0.0f && y > 0.0f)
			{
				float X = (Y / y) * x;
				float Z = (Y / y) * (1 - x - y);
				return GEAR_FLOAT3(X, Y, Z);
			}
			else
			{
				float X = (Y / Colour_Base_D65.y) * Colour_Base_D65.x;
				float Z = (Y / Colour_Base_D65.y) * (1 - Colour_Base_D65.x - Colour_Base_D65.y);
				return GEAR_FLOAT3(X, Y, Z);
			}
		}

		GEAR_FLOAT GetLuminance(GEAR_UINT type, GEAR_FLOAT3 CONST_REF value)
		{
			GEAR_FLOAT3 primaryR, primaryG, primaryB;

			switch (type)
			{
			default:
			case 0:
				primaryR = Colour_CIE_XYZ_PrimaryR;
				primaryG = Colour_CIE_XYZ_PrimaryG;
				primaryB = Colour_CIE_XYZ_PrimaryB;
			case 1:
				primaryR = Colour_sRGB_PrimaryR;
				primaryG = Colour_sRGB_PrimaryG;
				primaryB = Colour_sRGB_PrimaryB;
			case 2:
				primaryR = Colour_Rec709_PrimaryR;
				primaryG = Colour_Rec709_PrimaryG;
				primaryB = Colour_Rec709_PrimaryB;
			case 3:
				primaryR = Colour_Rec2020_PrimaryR;
				primaryG = Colour_Rec2020_PrimaryG;
				primaryB = Colour_Rec2020_PrimaryB;
			}

			return (value.r * primaryR.z) + (value.g * primaryG.z) + (value.b * primaryB.z);
		}

		GEAR_FLOAT sRGB_To_lsRGB(GEAR_FLOAT value)
		{
			float valuef = saturate(value);
			if (valuef <= 0.04045f)
				return valuef / 12.92f;
			else
				return pow((valuef + 0.055f) / 1.055f, 2.4f);
		}
		GEAR_FLOAT Rec709_To_lRec709(GEAR_FLOAT value)
		{
			float valuef = saturate(value);
			if (valuef < 0.081f)
				return valuef / 4.5f;
			else
				return pow(((valuef + 0.099f) / 1.099f), (1.0f / 0.45f));
		}
		GEAR_FLOAT Rec2020_to_lRec2020(GEAR_FLOAT value)
		{
			float valuef = saturate(value);
			if (valuef < 0.081f)
				return valuef / 4.5f;
			else
				return pow(((valuef + (1.09929682680944f - 1.0f)) / 1.0992968268094f), (1.0f / 0.45f));
		}
		GEAR_FLOAT3 Linearise(GEAR_UINT type, GEAR_FLOAT3 CONST_REF value)
		{
			switch (type)
			{
			default:
			case 0:
				return value;
			case 1:
				return GEAR_FLOAT3(sRGB_To_lsRGB(value.r), sRGB_To_lsRGB(value.g), sRGB_To_lsRGB(value.b));
			case 2:
				return GEAR_FLOAT3(Rec709_To_lRec709(value.r), Rec709_To_lRec709(value.g), Rec709_To_lRec709(value.b));
			case 3:
				return GEAR_FLOAT3(Rec2020_to_lRec2020(value.r), Rec2020_to_lRec2020(value.g), Rec2020_to_lRec2020(value.b));
			}
		}

		GEAR_FLOAT lsRGB_To_sRGB(GEAR_FLOAT value)
		{
			float resultf;
			if (value <= 0.0031308f)
				resultf = value * 12.92f;
			else
				resultf = 1.055f * pow(value, 1.0f / 2.4f) - 0.055f;

			return saturate(resultf);
		}
		GEAR_FLOAT lRec709_To_Rec709(GEAR_FLOAT value)
		{
			float resultf;
			if (value < 0.018)
				resultf = value * 4.5f;
			else
				resultf = 1.099f * pow(value, 0.45f) - 0.099f;

			return saturate(resultf);
		}
		GEAR_FLOAT lRec2020_To_Rec2020(GEAR_FLOAT value)
		{
			float resultf;
			if (value < 0.018053968510807f)
				resultf = value * 4.5f;
			else
				resultf = 1.09929682680944f * pow(value, 0.45f) - (1.09929682680944f - 1.0f);

			return saturate(resultf);
		}
		GEAR_FLOAT3 GammaCorrection(GEAR_UINT type, GEAR_FLOAT3 CONST_REF value)
		{
			switch (type)
			{
			default:
			case 0:
				return value;
			case 1:
				return GEAR_FLOAT3(lsRGB_To_sRGB(value.r), lsRGB_To_sRGB(value.g), lsRGB_To_sRGB(value.b));
			case 2:
				return GEAR_FLOAT3(lRec709_To_Rec709(value.r), lRec709_To_Rec709(value.g), lRec709_To_Rec709(value.b));
			case 3:
				return GEAR_FLOAT3(lRec2020_To_Rec2020(value.r), lRec2020_To_Rec2020(value.g), lRec2020_To_Rec2020(value.b));
			}
		}

#ifndef __cplusplus
		struct ColourTransformMatrics
		{
			GEAR_FLOAT3X3 RGB_To_XYZ;
			GEAR_FLOAT3X3 XYZ_To_RGB;
		};
		ColourTransformMatrics GetColourTransformMatrics(GEAR_UINT type)
		{
			GEAR_FLOAT3 primaryR, primaryG, primaryB;

			switch (type)
			{
			default:
			case 0:
				primaryR = Colour_CIE_XYZ_PrimaryR;
				primaryG = Colour_CIE_XYZ_PrimaryG;
				primaryB = Colour_CIE_XYZ_PrimaryB;
			case 1:
				primaryR = Colour_sRGB_PrimaryR;
				primaryG = Colour_sRGB_PrimaryG;
				primaryB = Colour_sRGB_PrimaryB;
			case 2:
				primaryR = Colour_Rec709_PrimaryR;
				primaryG = Colour_Rec709_PrimaryG;
				primaryB = Colour_Rec709_PrimaryB;
			case 3:
				primaryR = Colour_Rec2020_PrimaryR;
				primaryG = Colour_Rec2020_PrimaryG;
				primaryB = Colour_Rec2020_PrimaryB;
			}

			ColourTransformMatrics result;
			GEAR_FLOAT3X3 primaryMatrix = GEAR_FLOAT3X3(Colour_Base_xyY_To_XYZ(primaryR), Colour_Base_xyY_To_XYZ(primaryG), Colour_Base_xyY_To_XYZ(primaryB));
			result.RGB_To_XYZ = transpose(primaryMatrix);
			result.XYZ_To_RGB = inverse(result.RGB_To_XYZ);
			return result;
		}

		GEAR_FLOAT3 ToCIE_XYZ(GEAR_UINT type, GEAR_FLOAT3 value)
		{
			return mul(transpose(GetColourTransformMatrics(type).RGB_To_XYZ), Linearise(type, value));
		}
		GEAR_FLOAT3 FromCIE_XYZ(GEAR_UINT type, GEAR_FLOAT3 value)
		{
			return GammaCorrection(type, mul(transpose(GetColourTransformMatrics(type).XYZ_To_RGB), value));
		}
#endif
#ifdef __cplusplus
	}
}
#endif

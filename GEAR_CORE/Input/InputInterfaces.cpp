#include "gear_core_common.h"
#include "Input/InputInterfaces.h"

#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#include <combaseapi.h>
#include <xinput.h>
#endif

using namespace gear;
using namespace input;

InputInterface::API InputInterface::s_API = InputInterface::API::UNKNOWN;

InputInterface::InputInterface(CreateInfo* pCreateInfo)
	:m_CI(*pCreateInfo)
{
	if (m_CI.inputAPI != InputInterface::API::XINPUT)
		return;

	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		GEAR_FATAL(ErrorCode::INPUT | ErrorCode::INIT_FAILED, "Failed to CoInitialise XInput.");
	}

	XINPUT_STATE state;
	if (XInputGetState(m_CI.controllerIndex, &state) == ERROR_SUCCESS)
	{
		XINPUT_BATTERY_INFORMATION batInfo;
		XInputGetBatteryInformation(m_CI.controllerIndex, BATTERY_DEVTYPE_GAMEPAD, &batInfo);

		XINPUT_CAPABILITIES capab;
		XInputGetCapabilities(m_CI.controllerIndex, XINPUT_FLAG_GAMEPAD, &capab);
		
		if (arc::BitwiseCheck(capab.Flags, (WORD)XINPUT_CAPS_VOICE_SUPPORTED))
		{
			PWCHAR renderId = nullptr;
			PWCHAR captureId = nullptr;
			UINT rcount = 0;
			UINT ccount = 0;
			if (XInputGetAudioDeviceIds(m_CI.controllerIndex, nullptr, &rcount, nullptr, &ccount) == ERROR_SUCCESS)
			{
				renderId = new WCHAR[rcount];
				captureId = new WCHAR[ccount];
				XInputGetAudioDeviceIds(m_CI.controllerIndex, renderId, &rcount, captureId, &ccount);
				delete[] renderId;
				delete[] captureId;

			}
		}

		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed = vib.wRightMotorSpeed = WORD(float(UINT16_MAX) * 0.25f);
		XInputSetState(m_CI.controllerIndex, &vib);

		vib.wLeftMotorSpeed = vib.wRightMotorSpeed = 0;
		XInputSetState(m_CI.controllerIndex, &vib);
	}
}

InputInterface::~InputInterface()
{
}

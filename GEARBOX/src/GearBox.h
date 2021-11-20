#pragma once
#include "Core/Application.h"
#include "Core/EntryPoint.h"

class GEARBOX final : public gear::core::Application
{
protected:
	void Run() override;

private:
	static GEARBOX* s_App;

	void InternalRun();
	bool m_AllowReRun = true;

public:
	static GEARBOX* GetGEARBOX() { return s_App; }
	inline bool& GetAllowReRun() { return m_AllowReRun; }
};
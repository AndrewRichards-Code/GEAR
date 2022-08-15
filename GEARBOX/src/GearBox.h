#pragma once
#include "Core/Application.h"
#include "Core/EntryPoint.h"

class GEARBOX final : public gear::core::Application
{
protected:
	void Run() override;
};
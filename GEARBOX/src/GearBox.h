#pragma once
#include "Core/Application.h"
#include "Core/EntryPoint.h"

class GEARBOX final : public gear::core::Application
{
public:
	GEARBOX(const gear::core::ApplicationContext& context);
	~GEARBOX() = default;

protected:
	void Run() override;
};
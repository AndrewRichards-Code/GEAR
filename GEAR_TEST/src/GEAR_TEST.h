#pragma once
#include "Core/Application.h"
#include "Core/EntryPoint.h"

class GEAR_TEST final : public gear::core::Application
{
public:
	GEAR_TEST(const gear::core::ApplicationContext& context);
	~GEAR_TEST() = default;

protected:
	void Run() override;
};

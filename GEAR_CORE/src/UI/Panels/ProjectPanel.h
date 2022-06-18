#pragma once
#include "UI/Panels/BasePanel.h"

namespace gear
{
	namespace ui
	{
		namespace panels
		{
			class GEAR_API ProjectPanel final : public Panel
			{
				//enums/structs
			public:

				//Methods
			public:
				ProjectPanel();
				~ProjectPanel();

				void Draw() override;

				//Members
			private:
			};
		}
	}
}

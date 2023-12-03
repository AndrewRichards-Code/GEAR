#pragma once
#include "UI/Panels/BasePanel.h"
#include <deque>

namespace gear
{
	namespace ui
	{
		namespace panels
		{
			class GEAR_API OutputPanel final : public Panel
			{
				//struct/enum
			public:
				struct Entry
				{
					std::string date;
					std::string time;
					std::string loggerName;
					std::string logLevel;
					std::string fileAndLine;
					std::string functionSignature;
					std::string errorCode;
					std::string message;
					std::string rawString;
				};

				//Methods
			public:
				OutputPanel();
				~OutputPanel();

				void Draw() override;
			
			private:
				static void DebugMessageCallback(const std::string& string);

				//Members
			private:
				static std::deque<Entry> s_OutputEntries;
			};
		}
	}
}
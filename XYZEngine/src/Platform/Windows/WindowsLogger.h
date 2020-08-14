#pragma once
#include "XYZ/Core/Logger.h"


namespace XYZ {

/**	@class WindowsLogger
 *	Enables logging to console on Windows
 *
 */
	class WindowsLogger : public Logger
	{
	public:
		WindowsLogger();
		virtual void SetColor(const int color) override;

	private:
		enum WindowsColor
		{
			COLOR_BLUE_F = 1,
			COLOR_GREEN_F = 2,
			COLOR_RED_F = 4,
			COLOR_PURPLE_F = 5,
			COLOR_YELLOW_F = 6,
			COLOR_WHITE_F = 15,
		};
	private:
		void* m_Handle;
	};
}
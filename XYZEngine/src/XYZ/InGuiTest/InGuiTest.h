#pragma once
#include "InGuiElement.h"
#include "InGuiAllocator.h"

namespace XYZ {

	class IG
	{
	public:
		static void BeginFrame();
		static void EndFrame();

		static void BeginUI(size_t handle);
		static void EndUI();
		static void Separator();

		size_t AllocateUI(const std::initializer_list<IGElementType>& types, size_t** handles);


		template <IGElementType, typename ...Args>
		static IGReturnType UI(size_t handle, const char* label, Args&& ...args);

		static void End(size_t handle);
			
	};
}
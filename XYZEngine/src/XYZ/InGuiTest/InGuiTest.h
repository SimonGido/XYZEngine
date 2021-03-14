#pragma once
#include "InGuiElement.h"
#include "InGuiAllocator.h"

#include "XYZ/Event/Event.h"

namespace XYZ {

	class IG
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame(const glm::mat4& viewProjectionMatrix);
		static void EndFrame();
		static void OnEvent(Event& event);

		static void BeginUI(size_t handle);
		static void EndUI();
		static void Separator();

		static size_t AllocateUI(const std::initializer_list<IGElementType>& types, size_t** handles);


		template <IGElementType, typename ...Args>
		static IGReturnType UI(size_t handle, const char* label, Args&& ...args);
	
		template <IGElementType>
		static uint8_t GetFlags(size_t handle);

		static void End(size_t handle);
			
	};

}
#pragma once
#include "InGuiElement.h"
#include "InGuiUIElements.h"
#include "InGuiAllocator.h"
#include "InGuiContext.h"

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

		static std::pair<size_t, size_t> AllocateUI(const std::initializer_list<IGHierarchy>& hierarchy, size_t** handles);


		template <typename T, typename ...Args>
		static IGReturnType UI(size_t handle, const char* label, Args&& ...args);

		template <typename T>
		static T& GetUI(size_t handle)
		{
			getContext().RenderData.Rebuild = true;
			return getContext().Allocator.Get<T>(s_PoolHandle, handle);
		}

		template <typename T>
		static const T& GetUI(size_t handle)
		{
			return getContext().Allocator.Get<T>(s_PoolHandle, handle);
		}

		static void End(size_t handle);

	private:
		static IGContext& getContext();
	};

}
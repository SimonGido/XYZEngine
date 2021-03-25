#pragma once
#include "IGElement.h"
#include "IGUIElements.h"
#include "IGAllocator.h"
#include "IGContext.h"

#include "XYZ/Event/Event.h"

namespace XYZ {

	class IG
	{
	public:
		static void Init();
		static void Shutdown();
		static void LoadLayout(const char* filepath);

		static void BeginFrame(const glm::mat4& viewProjectionMatrix);
		static void EndFrame();
		static void OnEvent(Event& event);

		static void BeginUI(size_t handle);
		static void EndUI();
		static void Separator();

		static std::pair<size_t, size_t> AllocateUI(const std::initializer_list<IGHierarchyElement>& hierarchy, size_t** handles);


		template <typename T, typename ...Args>
		static IGReturnType UI(size_t handle, Args&& ...args);

		template <typename T>
		static T& GetUI(size_t poolHandle,size_t handle)
		{
			getContext().RenderData.Rebuild = true;
			IGElement* element = getContext().Allocator.Get<IGElement>(poolHandle, handle);
			T* result = dynamic_cast<T*>(element);
			XYZ_ASSERT(result, "");
			return *result;
		}

		static void End(size_t handle);

	private:
		static IGContext& getContext();
	};

}
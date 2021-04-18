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
		static void RebuildUI();


		static std::pair<size_t, size_t> AllocateUI(const std::initializer_list<IGHierarchyElement>& hierarchy);
		static std::pair<size_t, size_t> AllocateUI(const std::vector<IGHierarchyElement>& hierarchy);
		static size_t ReallocateUI(size_t handle, const std::vector<IGHierarchyElement>& hierarchy);

		template <typename T>
		static void ForEach(size_t poolHandle, const std::function<void(T&)>& callback)
		{
			IGPool& pool = GetContext().Allocator.GetPools()[poolHandle];
			for (size_t i = 0; i < pool.Size(); ++i)
			{
				T* elem = dynamic_cast<T*>(pool[i]);
				if (elem)
					callback(*elem);
			}
		}
		template <typename T>
		static T& GetUI(size_t poolHandle, size_t index)
		{
			GetContext().RenderData.Rebuild = true;
			IGElement* element = GetContext().Allocator.GetPools()[poolHandle][index];
			T* result = dynamic_cast<T*>(element);
			XYZ_ASSERT(result, "");
			return *result;
		}


		static IGContext& GetContext();
	};

}
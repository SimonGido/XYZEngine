#pragma once

#include <glm/glm.hpp>


struct GLFWwindow;
namespace XYZ {
	class APIContext : public RefCount
	{
	public:
		virtual ~APIContext() = default;

		virtual void Init() = 0;
		virtual void CreateSwapChain(uint32_t* width, uint32_t* height, bool vSync) {};
		virtual void SwapBuffers() = 0;

		static Ref<APIContext> Create(void* window);
	};
}
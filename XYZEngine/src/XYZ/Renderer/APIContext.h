#pragma once

#include <glm/glm.hpp>


struct GLFWwindow;
namespace XYZ {
	class APIContext : public RefCount
	{
	public:
		virtual ~APIContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static Ref<APIContext> Create(void* window);
	};
}
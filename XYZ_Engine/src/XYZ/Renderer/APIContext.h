#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>
#include <memory>
#include <functional>


/**
* @interface APIContext
* pure virtual (interface) class.
*/
namespace XYZ {
	class APIContext
	{
	public:
		/**	
		* Initialize graphics API context
		* @return void
		*/
		virtual void Init() = 0;

		/*
		* Swap window render buffer of the current graphics API
		* @return void
		*/
		virtual void SwapBuffers() = 0;

		/*
		* Creates APIContext dependent on the graphics API 
		* param[in] window     Pointer to the window handler
		* @return a unique pointer to APIContext
		*/
		static std::unique_ptr<APIContext> Create(void* window);
	};
}
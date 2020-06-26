#pragma once

#include "OrthoCamera.h"
#include "RendererBatchSystem2D.h"
#include "RenderCommandQueue.h"


namespace XYZ {
	class Renderer2D
	{
	public:
		/**
		* Initialize Renderer2D instance and register batch system
		*/
		static void Init();

		/**
		* Delete instance of the renderer
		*/
		static void Shutdown();

		/**
		* Initial setup before rendering
		* @param[in] camera		Reference to the camera 
		*/
		static void BeginScene(const OrthoCamera& camera);

		/**
		* Submit command to the command queue
		* @param[in] command	Reference to the command
		* @param[in] size		Size of the command
		*/
		static void Submit(CommandI& command, unsigned int size);

		/**
		* Execute the command queue
		*/
		static void Flush();

		/**
		* Clean up after rendering
		*/
		static void EndScene();

	private:
		static Renderer2D* s_Instance;

		RenderCommandQueue m_CommandQueue;
		std::shared_ptr<RendererBatchSystem2D> m_BatchSystem;
	};
}
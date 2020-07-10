#pragma once
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "VertexArray.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"


namespace XYZ {

	struct RendererSubmitData
	{
		const std::shared_ptr<Shader> m_Shader;
		const std::shared_ptr<VertexArray> m_VertexArray;

		RendererSubmitData(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray> vertexArray) : m_Shader(shader), m_VertexArray(vertexArray) {};
	};

	/**
	* @class Renderer
	* @brief represents encapsulation for systems, that takes care of sorting and rendering objects
	*/
	class Renderer
	{
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};
	public:
		/**
		* Initialize RenderCommand and Renderer2D
		*/
		static void Init();

		/**
		* Function handling window resizing
		* @param[in] width   Width of the window
		* @param[in] height  Height of the window
		*/
		static void OnWindowResize(uint32_t width, uint32_t height);

		/**
		* @return RendererAPI
		*/
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }


		/**
		* Initial setup before rendering
		* @param[in] camera		Reference to the camera
		*/
		static void BeginScene(Camera& camera);

		/**
		* Clean up after rendering
		*/
		static void EndScene();


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
	private:
		static Renderer* s_Instance;

		static SceneData* s_SceneData;
		RenderCommandQueue m_CommandQueue;
	};

}
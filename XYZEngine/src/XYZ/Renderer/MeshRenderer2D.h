#pragma once

#include "Camera.h"
#include "RenderCommandQueue.h"
#include "Material.h"
#include "Mesh.h"


namespace XYZ {

	struct SceneMeshRenderData
	{
		glm::mat4 ViewProjectionMatrix;
		glm::vec2 ViewportSize;
	};

	class MeshRenderer2D
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
		static void BeginScene(const SceneMeshRenderData& data);
		static void SetMaterial(const Ref<Material>& material);

		//static void SubmitMesh(const glm::mat4& transform, const Ref<Mesh>& mesh);
		/**
		* Execute the command queue
		*/
		static void Flush();
		/**
		* Clean up after rendering
		*/
		static void EndScene();

	};
}
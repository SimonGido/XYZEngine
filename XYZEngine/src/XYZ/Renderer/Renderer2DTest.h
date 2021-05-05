#pragma once

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "Buffer.h"

namespace XYZ {
	
	class Renderer2DLayout : public RefCount
	{
	public:
		Renderer2DLayout(const BufferLayout& layout);

		void SetBufferLayout(const BufferLayout& layout);

		void SetQuadMaterial(const Ref<Material>& material);
		void SetLineShader(const Ref<Shader>& lineShader);

	private:
		Ref<VertexArray>  m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Material>     m_QuadMaterial;
		Ref<Shader>		  m_LineShader;
		
		friend class Renderer2DTest;
	};

	class Renderer2DTest
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const glm::mat4& viewProjectionMatrix, const Ref<Renderer2DLayout>& layout);

		template <typename ...Args>
		static void SubmitQuad(const glm::mat4& transform, Args&& ...args)
		{
			constexpr size_t quadVertexCount = 4;
			if (s_Data.IndexCount >= s_Data.MaxIndices)
				Flush();
		}
		template <typename ...Args>
		static void SubmitQuad(const glm::vec3& position, const glm::vec2& size, Args&& ...args)
		{

		}

		static void Flush();

	private:

	};
}
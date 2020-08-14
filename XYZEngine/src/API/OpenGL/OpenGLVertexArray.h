#pragma once
#include "XYZ/Renderer/VertexArray.h"


namespace XYZ {
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void AddShaderStorageBuffer(const Ref<ShaderStorageBuffer>& shaderBuffer)override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual inline const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; };
		virtual inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; };
	
	private:
		uint32_t m_RendererID;
		Ref<IndexBuffer> m_IndexBuffer;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		std::vector<Ref<ShaderStorageBuffer>> m_ShaderStorageBuffers;
	};

}
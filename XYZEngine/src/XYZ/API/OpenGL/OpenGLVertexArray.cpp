#include "stdafx.h"
#include "OpenGLVertexArray.h"


#include "XYZ/Renderer/Renderer.h"

#include <GL/glew.h>


namespace XYZ {
	static GLenum ShaderDataComponentToOpenGLBaseComponent(ShaderDataComponent Component)
	{
		switch (Component)
		{
		case ShaderDataComponent::Bool:   return GL_BOOL;
		case ShaderDataComponent::Float:  return GL_FLOAT;
		case ShaderDataComponent::Float2: return GL_FLOAT;
		case ShaderDataComponent::Float3: return GL_FLOAT;
		case ShaderDataComponent::Float4: return GL_FLOAT;
		case ShaderDataComponent::Int:    return GL_INT;
		case ShaderDataComponent::Int2:   return GL_INT;
		case ShaderDataComponent::Int3:   return GL_INT;
		case ShaderDataComponent::Int4:   return GL_INT;
		case ShaderDataComponent::Mat3:   return GL_FLOAT;
		case ShaderDataComponent::Mat4:   return GL_FLOAT;
		}

		XYZ_ASSERT(false, "ShaderDataComponentSize(ShaderDataComponent::None)");
		return GL_NONE;
	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		Ref<OpenGLVertexArray> instance = this;
		Renderer::Submit([instance]() mutable {
			glCreateVertexArrays(1, &instance->m_RendererID); 
		});
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		
	}

	void OpenGLVertexArray::Release() const
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteVertexArrays(1, &rendererID);
		});
	}

	void OpenGLVertexArray::Bind() const
	{
		Ref<const OpenGLVertexArray> instance = this;
		Renderer::Submit([instance]() {
			glBindVertexArray(instance->m_RendererID); 
		});
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		if (vertexBuffer->GetLayout().GetElements().size() == 0)
			XYZ_ASSERT(false, "vertexBuffer->GetLayout().GetElements().size() = 0");

		Ref<OpenGLVertexArray> instance = this;
		Renderer::Submit([instance, vertexBuffer]()mutable {
			glBindVertexArray(instance->m_RendererID);
		
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->GetRendererID());
			auto& vbl = vertexBuffer->GetLayout();
			for (const auto& element : vbl)
			{
				glEnableVertexAttribArray(element.Index);
				if (element.Component == ShaderDataComponent::Int 
				 || element.Component == ShaderDataComponent::Int2
				 || element.Component == ShaderDataComponent::Int3
				 || element.Component == ShaderDataComponent::Int4)
				{
					glVertexAttribIPointer(element.Index,
						element.GetComponentCount(),
						ShaderDataComponentToOpenGLBaseComponent(element.Component),
						vbl.GetStride(),
						(const void*)(uint64_t)element.Offset);
				}
				else
				{				
					glVertexAttribPointer(element.Index,
						element.GetComponentCount(),
						ShaderDataComponentToOpenGLBaseComponent(element.Component),
						//element.m_Normalized ? GL_TRUE : GL_FALSE,
						GL_FALSE,
						vbl.GetStride(),
						(const void*)(uint64_t)element.Offset);
					glVertexAttribDivisor(element.Index, element.Divisor);
				}
			}

			instance->m_VertexBuffers.push_back(vertexBuffer);
		});
	}

	void OpenGLVertexArray::AddShaderStorageBuffer(const Ref<ShaderStorageBuffer>& shaderBuffer)
	{
		if (shaderBuffer->GetLayout().GetElements().size() == 0)
			XYZ_ASSERT(false, "vertexBuffer->GetLayout().GetElements().size() = 0");

		Ref<OpenGLVertexArray> instance = this;
		Renderer::Submit([instance, shaderBuffer] ()mutable {
			glBindVertexArray(instance->m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, shaderBuffer->GetRendererID());
	
			auto& vbl = shaderBuffer->GetLayout();
			for (const auto& element : vbl)
			{
				glEnableVertexAttribArray(element.Index);
				if (element.Component == ShaderDataComponent::Int)
				{		
					glVertexAttribIPointer(element.Index,
						element.GetComponentCount(),
						ShaderDataComponentToOpenGLBaseComponent(element.Component),
						//element.m_Normalized ? GL_TRUE : GL_FALSE,
						vbl.GetStride(),
						(const void*)(uint64_t)element.Offset);
				}
				else
				{
					glVertexAttribPointer(element.Index,
						element.GetComponentCount(),
						ShaderDataComponentToOpenGLBaseComponent(element.Component),
						//element.m_Normalized ? GL_TRUE : GL_FALSE,
						GL_FALSE,
						vbl.GetStride(),
						(const void*)(uint64_t)element.Offset);
				}
				glVertexAttribDivisor(element.Index, element.Divisor);
			}

			instance->m_ShaderStorageBuffers.push_back(shaderBuffer);
			});
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		Ref<OpenGLVertexArray> instance = this;
		Renderer::Submit([instance, indexBuffer] () mutable {
			glBindVertexArray(instance->m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, indexBuffer->GetRendererID());
			});
	}

}
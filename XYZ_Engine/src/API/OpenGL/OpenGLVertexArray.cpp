#include "stdafx.h"
#include "OpenGLVertexArray.h"

#include <GL/glew.h>


namespace XYZ {
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Bool:   return GL_BOOL;
		case ShaderDataType::Float:  return GL_FLOAT;
		case ShaderDataType::Float2: return GL_FLOAT;
		case ShaderDataType::Float3: return GL_FLOAT;
		case ShaderDataType::Float4: return GL_FLOAT;
		case ShaderDataType::Int:    return GL_INT;
		case ShaderDataType::Int2:   return GL_INT;
		case ShaderDataType::Int3:   return GL_INT;
		case ShaderDataType::Int4:   return GL_INT;
		case ShaderDataType::Mat3:   return GL_FLOAT;
		case ShaderDataType::Mat4:   return GL_FLOAT;
		}

		XYZ_ASSERT(false, "ShaderDataTypeSize(ShaderDataType::None)");
		return GL_NONE;
	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		if (vertexBuffer->GetLayout().GetElements().size() == 0)
			XYZ_ASSERT(false, "vertexBuffer->GetLayout().GetElements().size() = 0");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		auto& vbl = vertexBuffer->GetLayout();
		for (const auto& element : vbl)
		{
			glEnableVertexAttribArray(element.Index);
			glVertexAttribPointer(element.Index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				//element.m_Normalized ? GL_TRUE : GL_FALSE,
				GL_FALSE,
				vbl.GetStride(),
				(const void*)element.Offset);
			glVertexAttribDivisor(element.Index, element.Divisor);
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::AddShaderStorageBuffer(const std::shared_ptr<ShaderStorageBuffer>& shaderBuffer)
	{
		if (shaderBuffer->GetLayout().GetElements().size() == 0)
			XYZ_ASSERT(false, "vertexBuffer->GetLayout().GetElements().size() = 0");

		glBindVertexArray(m_RendererID);
		shaderBuffer->Bind();

		auto& vbl = shaderBuffer->GetLayout();
		for (const auto& element : vbl)
		{
			glEnableVertexAttribArray(element.Index);
			glVertexAttribPointer(element.Index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				//element.m_Normalized ? GL_TRUE : GL_FALSE,
				GL_FALSE,
				vbl.GetStride(),
				(const void*)element.Offset);
			glVertexAttribDivisor(element.Index, element.Divisor);
		}

		m_ShaderStorageBuffers.push_back(shaderBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}
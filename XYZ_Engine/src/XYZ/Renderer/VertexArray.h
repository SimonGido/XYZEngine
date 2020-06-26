#pragma once
#include "Buffer.h"

namespace XYZ {
	/**
	* @interface VertexArray
	* pure virtual (interface) class.
	*/
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void AddShaderStorageBuffer(const std::shared_ptr<ShaderStorageBuffer>& shaderBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffer() const = 0;


		/**
		* @ return shared_ptr to VertexArray
		*/
		static std::shared_ptr<VertexArray> Create();
	};
}
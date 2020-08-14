#pragma once
#include "Buffer.h"

#include "XYZ/Core/Ref.h"

namespace XYZ {
	/**
	* @interface VertexArray
	* pure virtual (interface) class.
	*/
	class VertexArray : public RefCount
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void AddShaderStorageBuffer(const Ref<ShaderStorageBuffer>& shaderBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const = 0;


		/**
		* @ return shared_ptr to VertexArray
		*/
		static Ref<VertexArray> Create();
	};
}
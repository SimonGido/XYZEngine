#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Assert.h"

#include <memory>

namespace XYZ {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};


	static unsigned int ShaderDataTypeSize(ShaderDataType Component)
	{
		switch (Component)
		{
		case ShaderDataType::Float:	return 4;
		case ShaderDataType::Float2:	return 4 * 2;
		case ShaderDataType::Float3:	return 4 * 3;
		case ShaderDataType::Float4:	return 4 * 4;
		case ShaderDataType::Mat3:		return 4 * 3 * 3;
		case ShaderDataType::Mat4:		return 4 * 4 * 4;
		case ShaderDataType::Int:		return 4;
		case ShaderDataType::Int2:		return 4 * 2;
		case ShaderDataType::Int3:		return 4 * 3;
		case ShaderDataType::Int4:		return 4 * 4;
		case ShaderDataType::Bool:		return 1;
		}
		XYZ_ASSERT(false, "Buffer: Unknown ShaderDataType");
		return 0;
	}

	struct BufferElement
	{
		BufferElement(uint32_t index, ShaderDataType Component, const std::string& name, uint32_t divisor = 0)
			: Index(index), Component(Component), Divisor(divisor), Size(ShaderDataTypeSize(Component)), Offset(0)
		{}


		uint32_t GetComponentCount() const
		{
			switch (Component)
			{
			case ShaderDataType::Bool:   return 1;
			case ShaderDataType::Float:  return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::Int:    return 1;
			case ShaderDataType::Int2:   return 2;
			case ShaderDataType::Int3:   return 3;
			case ShaderDataType::Int4:   return 4;
			case ShaderDataType::Mat3:   return 9;
			case ShaderDataType::Mat4:   return 16;
			}
			XYZ_ASSERT(false, "ShaderDataTypeSize(ShaderDataType::None)");
			return 0;
		}

		ShaderDataType Component;
		uint32_t   Size;
		uint32_t   Offset;
		uint32_t   Index;
		uint32_t   Divisor;
	};

	class BufferLayout
	{
	public:
		BufferLayout() {};

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CreateMat4();
			CalculateOffsetsAndStride();
		}
		BufferLayout(const std::vector<BufferElement>& elements)
			: m_Elements(elements)
		{
			CreateMat4();
			CalculateOffsetsAndStride();
		}


		inline const uint32_t& GetStride() const { return m_Stride; }


		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		auto begin() { return m_Elements.begin(); }
		auto end() { return m_Elements.end(); }
		auto begin() const { return m_Elements.begin(); }
		auto end() const { return m_Elements.end(); }
	private:

		inline void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		};


		inline void CreateMat4()
		{
			for (auto& element : m_Elements)
			{
				if (element.Component == ShaderDataType::Mat4)
				{
					element.Component = ShaderDataType::Float4;
					element.Size = 4 * 4;

					BufferElement tmpElement = element;
					m_Elements.push_back(BufferElement(tmpElement.Index + 1, tmpElement.Component, "", tmpElement.Divisor));
					m_Elements.push_back(BufferElement(tmpElement.Index + 2, tmpElement.Component, "", tmpElement.Divisor));
					m_Elements.push_back(BufferElement(tmpElement.Index + 3, tmpElement.Component, "", tmpElement.Divisor));
				}
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};


	enum class BufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	class VertexBuffer : public RefCount
	{
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
		virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Resize(const void* vertices, uint32_t size) = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		static Ref<VertexBuffer> Create(uint32_t size);

		static Ref<VertexBuffer> Create(const void* vertices, uint32_t size, BufferUsage usage = BufferUsage::Static);
	};


	class IndexBuffer : public RefCount
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		static Ref<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
	};



	class ShaderStorageBuffer : public RefCount
	{
	public:
		virtual ~ShaderStorageBuffer() = default;
		virtual void BindBase(uint32_t binding) const = 0;
		virtual void BindRange(uint32_t offset, uint32_t size) const = 0;
		virtual void Bind() const = 0;
		
		virtual void Update(void* vertices, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Resize(void* vertices, uint32_t size) = 0;
		virtual void GetSubData(void** buffer, uint32_t size, uint32_t offset = 0) = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		

		static Ref<ShaderStorageBuffer> Create(uint32_t size, uint32_t binding);

		static Ref<ShaderStorageBuffer> Create(const float* vertices, uint32_t size, uint32_t binding, BufferUsage usage = BufferUsage::Dynamic);
	};


	class AtomicCounter : public RefCount
	{
	public:
		virtual ~AtomicCounter() = default;

		virtual void Reset() = 0;
		virtual void BindBase(uint32_t index) const = 0;
		virtual void Update(uint32_t* data, uint32_t count, uint32_t offset) = 0;
		virtual uint32_t* GetCounters() = 0;
		virtual uint32_t GetNumCounters() = 0;

		static Ref<AtomicCounter> Create(uint32_t size, uint32_t binding);
	};


	struct DrawArraysIndirectCommand
	{
		uint32_t Count;
		uint32_t InstanceCount;
		uint32_t FirstVertex;
		uint32_t BaseInstance;
	};

	struct DrawElementsIndirectCommand
	{
		uint32_t Count;         
		uint32_t InstanceCount; 
		uint32_t FirstIndex;    
		uint32_t BaseVertex;    
		uint32_t BaseInstance;  
	};
	

	class IndirectBuffer : public RefCount
	{
	public:
		virtual ~IndirectBuffer() = default;
		
		virtual void Bind() const = 0;
		virtual void BindBase(uint32_t index) = 0;

		static Ref<IndirectBuffer> Create(void * drawCommand, uint32_t size, uint32_t binding);
	};


	class UniformBuffer : public RefCount
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void Update(const void* data, uint32_t size, uint32_t offset) = 0;


		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}
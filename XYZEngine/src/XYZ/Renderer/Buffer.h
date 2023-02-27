#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Assert.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <memory>

namespace XYZ {

	enum class ShaderDataType
	{
		None = 0, 
		Float, 
		Float2, 
		Float3, 
		Float4, 
		Mat3, 
		Mat4, 
		Int, 
		Int2, 
		Int3, 
		Int4, 
		Bool
	};


	static unsigned int ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
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

	struct XYZ_API BufferElement
	{
		BufferElement(uint32_t location, ShaderDataType type, const std::string_view name)
			: Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Location(location)
		{}

		ShaderDataType Type;
		uint32_t	   Size;
		uint32_t	   Offset;
		uint32_t	   Location;
	};

	class XYZ_API BufferLayout
	{
	public:
		BufferLayout(bool instanced = false)
		: m_Instanced(instanced) {};

		BufferLayout(const std::initializer_list<BufferElement>& elements, bool instanced = false)
			: m_Elements(elements), m_Instanced(instanced)
		{
			calculateOffsetsAndStride();
		}
		BufferLayout(const std::vector<BufferElement>& elements, bool instanced = false)
			: m_Elements(elements), m_Instanced(instanced)
		{
			calculateOffsetsAndStride();
		}


		inline const uint32_t& GetStride() const { return m_Stride; }


		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline const bool Empty() const { return m_Elements.empty(); }
		inline const bool Instanced() const { return m_Instanced; }

		auto begin() { return m_Elements.begin(); }
		auto end() { return m_Elements.end(); }
		auto begin() const { return m_Elements.begin(); }
		auto end() const { return m_Elements.end(); }
	private:

		inline void calculateOffsetsAndStride()
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

	private:
		std::vector<BufferElement> m_Elements;
		bool					   m_Instanced;
		uint32_t				   m_Stride = 0;

	};


	enum class BufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	class XYZ_API VertexBuffer : public RefCount
	{
	public:
		virtual ~VertexBuffer() = default;
		
		// Old API
		virtual void Bind() const {}
		virtual void UnBind() const {};
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual void Resize(const void* vertices, uint32_t size) = 0;

		// New API
		virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) = 0;
		virtual void RT_Update(const void* vertices, uint32_t size, uint32_t offset = 0) {};
		virtual void SetUseSize(uint32_t size) = 0;
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetUseSize() const = 0;
		
		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(const void* vertices, uint32_t size);
	};


	enum class IndexType
	{
		Uint8, Uint16, Uint32
	};

	class XYZ_API IndexBuffer : public RefCount
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void Update(const void* indices, uint32_t count, uint32_t offset = 0) {};
		virtual void RT_Update(const void* indices, uint32_t count, uint32_t offset = 0) {};
		virtual void SetUseCount(uint32_t count) = 0;
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetUseCount() const = 0;
		virtual IndexType GetIndexType() const { return IndexType::Uint32; }

		static Ref<IndexBuffer> Create(const void* indices, uint32_t count, IndexType type = IndexType::Uint32);
	};



	class XYZ_API StorageBuffer : public RefCount
	{
	public:
		virtual ~StorageBuffer() = default;
		// Old API
		virtual void BindBase(uint32_t binding) const {};
		virtual void BindRange(uint32_t offset, uint32_t size) const {};
		virtual void Bind() const {};
		virtual void GetSubData(void** buffer, uint32_t size, uint32_t offset = 0) {};
		virtual void SetLayout(const BufferLayout& layout) {};
		virtual uint32_t GetRendererID() const { return 0; };
		//

		virtual void Update(const void* data, uint32_t size, uint32_t offset = 0) {};
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset = 0) {};
		virtual void Update(ByteBuffer data, uint32_t size, uint32_t offset = 0) {}
		virtual void Resize(uint32_t size) {};
		virtual void SetBufferInfo(uint32_t size, uint32_t offset) {};

		virtual uint32_t			GetBinding() const { return 0; }
		virtual const BufferLayout& GetLayout() const { return BufferLayout(); };
		virtual ByteBuffer			GetBuffer() { return ByteBuffer(); }
		virtual bool				IsIndirect() const { return false; }
		
		static Ref<StorageBuffer> Create(uint32_t size, uint32_t binding, bool indirect = false);
		static Ref<StorageBuffer> Create(const void *data, uint32_t size, uint32_t binding, bool indirect = false, BufferUsage usage = BufferUsage::Dynamic);
	};


	class XYZ_API AtomicCounter : public RefCount
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


	struct XYZ_API IndirectDrawCommand
	{
		uint32_t Count;
		uint32_t InstanceCount;
		uint32_t FirstVertex;
		uint32_t BaseInstance;
	};

	struct XYZ_API IndirectIndexedDrawCommand
	{
		uint32_t Count			= 0;         
		uint32_t InstanceCount	= 0;
		uint32_t FirstIndex		= 0;    
		uint32_t BaseVertex		= 0;    
		uint32_t BaseInstance	= 0;

	private:
		Padding<12> Padding;
	};
	

	class XYZ_API IndirectBuffer : public RefCount
	{
	public:
		virtual ~IndirectBuffer() = default;
		
		virtual void Bind() const {};
		virtual void BindBase(uint32_t index) {};

		virtual uint32_t GetBinding() const { return 0; }

		static Ref<IndirectBuffer> Create(const void * data, uint32_t size, uint32_t binding);
	};


	class XYZ_API UniformBuffer : public RefCount
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void Update(const void* data, uint32_t size, uint32_t offset) = 0;
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset) {};

		virtual uint32_t GetBinding() const = 0;
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}
#pragma once
#include "XYZ/Core/Ref.h"

#include <memory>

namespace XYZ {

	/**
	* Represents data Components in shader program
	*/
	enum class ShaderDataComponent
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	/**
	* Return size of specified shader data Component
	* @param[in] Component   ShaderDataComponent
	* @return a size of the Component
	*/
	static unsigned int ShaderDataComponentSize(ShaderDataComponent Component)
	{
		switch (Component)
		{
		case ShaderDataComponent::Float:		return 4;
		case ShaderDataComponent::Float2:	return 4 * 2;
		case ShaderDataComponent::Float3:	return 4 * 3;
		case ShaderDataComponent::Float4:	return 4 * 4;
		case ShaderDataComponent::Mat3:		return 4 * 3 * 3;
		case ShaderDataComponent::Mat4:		return 4 * 4 * 4;
		case ShaderDataComponent::Int:		return 4;
		case ShaderDataComponent::Int2:		return 4 * 2;
		case ShaderDataComponent::Int3:		return 4 * 3;
		case ShaderDataComponent::Int4:		return 4 * 4;
		case ShaderDataComponent::Bool:		return 1;
		}
		XYZ_ASSERT(false, "Buffer: Unknown ShaderDataComponent");
		return 0;
	}

	/**
	* @struct BufferElement
	* Store information about buffer element
	* 
	* Each element contains information about it's size in vertex buffer.
	* The vertex buffers can store only raw data, the buffer element let us use
	* custom ShaderDataComponent values in the vertex buffers.
	*/
	struct BufferElement
	{
		/**
		* Constructor
		* @param[in] index		Index of element in buffer
		* @param[in] Component		Shader data Component
		* @param[in] name		Name of element represented in shader
		* @param[in] divisior	Specify how is data split between instances, default 0
		*/
		BufferElement(uint32_t index, ShaderDataComponent Component, const std::string& name, uint32_t divisor = 0)
			: Index(index), Component(Component), Divisor(divisor), Size(ShaderDataComponentSize(Component)), Offset(0)
		{}

		/**
		* Split ShaderDataComponent to four byte values and return count
		* @return a count of four byte values after split
		*/
		uint32_t GetComponentCount() const
		{
			switch (Component)
			{
			case ShaderDataComponent::Bool:   return 1;
			case ShaderDataComponent::Float:  return 1;
			case ShaderDataComponent::Float2: return 2;
			case ShaderDataComponent::Float3: return 3;
			case ShaderDataComponent::Float4: return 4;
			case ShaderDataComponent::Int:    return 1;
			case ShaderDataComponent::Int2:   return 2;
			case ShaderDataComponent::Int3:   return 3;
			case ShaderDataComponent::Int4:   return 4;
			case ShaderDataComponent::Mat3:   return 9;
			case ShaderDataComponent::Mat4:   return 16;
			}
			XYZ_ASSERT(false, "ShaderDataComponentSize(ShaderDataComponent::None)");
			return 0;
		}

		ShaderDataComponent Component;
		uint32_t   Size;
		uint32_t   Offset;
		uint32_t   Index;
		uint32_t   Divisor;
	};


	/**
	* @class BufferLayout
	* Represents layout of data in buffer. Consists of multiple BufferElements, stored in vector,
	* let us structure data in the vertex buffer.
	*/
	class BufferLayout
	{
	public:
		/**
		* default Constructor
		*/
		BufferLayout() {};

		/**
		* Takes initializer_list of BufferElements.
		* Generates special Mat4 buffer elements, calcultes offsets and strides in elements
		* @param[in] elements initializer_list of BufferElements 
		*/
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

		/**
		* @return a size of the BufferElements in bytes
		*/
		inline const uint32_t& GetStride() const { return m_Stride; }

		/**
		* @return a vector of stored BufferElements
		*/
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		auto begin() { return m_Elements.begin(); }
		auto end() { return m_Elements.end(); }
		auto begin() const { return m_Elements.begin(); }
		auto end() const { return m_Elements.end(); }
	private:

		/**
		* Calculate offsets and strides
		*
		* Calculate offset in the vertex buffer for each buffer element,
		* and size of the BufferElements in bytes
		*/
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

		/**
		* If element Component equals ShaderDataComponent::Mat4 ,it must create three additional elements,
		* and set for the element and the three additional elements ShaderDataComponent::Float4.
		*/
		inline void CreateMat4()
		{
			for (auto& element : m_Elements)
			{
				if (element.Component == ShaderDataComponent::Mat4)
				{
					element.Component = ShaderDataComponent::Float4;
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

	/**
	* Represents usage of the vertex buffer
	* If Static, the data is not expected to be updated,
	* if Dynamic, the data is expected to be updated
	*/
	enum class BufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	/**
	* @interface VertexBuffer
	* pure virtual (interface) class.
	* Storage of data for rendering. Send the data to the GPU for further processing. Vertices are rendered by shader program.
	* VertexArray stores VertexBuffers, must have BufferLayout set before being stored.
	*/
	class VertexBuffer : public RefCount
	{
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
		virtual void Update(void* vertices, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Resize(float* vertices, uint32_t size) = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		/**
		* Create empty VertexBuffer, Buffer usage is Dynamic
		* @param size	Size of the buffer in bytes
		* @return shared_ptr to VertexBuffer
		*/
		static Ref<VertexBuffer> Create(uint32_t size);

		/**
		* Create VertexBuffer
		* @param[in] vertices  Pointer to the vertices
		* @param[in] size	Size of the buffer in bytes
		* @param[in] usage  Data in the buffer will be static or dynamic , default Static
		* @return shared_ptr to VertexBuffer
		*/
		static Ref<VertexBuffer> Create(void* vertices, uint32_t size, BufferUsage usage = BufferUsage::Static);
	};

	/**
	* @interface IndexBuffer
	* pure virtual (interface) class.
	* Storage of the indices, send them to the GPU for further processing. 
	* The GPU use stored indices for indexing the vertices in the vertex buffer
	*/
	class IndexBuffer : public RefCount
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		/**
		* Create IndexBuffer
		* @param[in] indices	Pointer to the indices
		* @param[in] count		Count of the indices
		* @return shared_ptr to IndexBuffer
		*/
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};


	/**
	* @interface ShaderStorageBuffer
	* pure virtual (interface) class.
	* Storage of the data, can be processed by compute shaders.
	*/
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
		
		/**
		* Create empty ShaderStorageBuffer, Buffer usage is Dynamic
		* @param[in] size	Size of the buffer in bytes
		* @return shared_ptr to ShaderStorageBuffer
		*/
		static Ref<ShaderStorageBuffer> Create(uint32_t size, uint32_t binding);

		/**
		* Create ShaderStorageBuffer
		* @param[in] vertices  Pointer to the vertices
		* @param[in] size	Size of the buffer in bytes
		* @param[in] usage  Data in the buffer will be static or dynamic , default Dynamic
		* @return shared_ptr to ShaderStoageBuffer
		*/
		static Ref<ShaderStorageBuffer> Create(float* vertices, uint32_t size, uint32_t binding, BufferUsage usage = BufferUsage::Dynamic);
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
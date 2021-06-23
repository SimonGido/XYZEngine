#pragma once
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum class ParticleVertexStreamType
	{
		Position,
		Velocity,
		Color,
		TexCoord
	};

	static uint32_t ParticleVertexStreamTypeToSize(ParticleVertexStreamType type)
	{
		switch (type)
		{
		case XYZ::ParticleVertexStreamType::Position:
			return (uint32_t)sizeof(glm::vec3);
			break;
		case XYZ::ParticleVertexStreamType::Color:
			return (uint32_t)sizeof(glm::vec4);
			break;
		case XYZ::ParticleVertexStreamType::TexCoord:
			return (uint32_t)sizeof(glm::vec4);
			break;
		default:
			break;
		}
	}

	static ShaderDataComponent ParticleVertexStreamTypeToShaderDataType(ParticleVertexStreamType type)
	{
		switch (type)
		{
		case XYZ::ParticleVertexStreamType::Position:
			return ShaderDataComponent::Float3;
			break;
		case XYZ::ParticleVertexStreamType::Color:
			return ShaderDataComponent::Float4;
			break;
		case XYZ::ParticleVertexStreamType::TexCoord:
			return ShaderDataComponent::Float4;
			break;
		}
		return ShaderDataComponent::None;
	}

	class ParticleVertexStream
	{
	public:
		ParticleVertexStream(ParticleVertexStreamType type, uint32_t capacity);
		ParticleVertexStream(ParticleVertexStream&& other) noexcept;
		ParticleVertexStream(const ParticleVertexStream& other);
		~ParticleVertexStream();

		ParticleVertexStream& operator=(ParticleVertexStream&& other) noexcept;
		ParticleVertexStream& operator= (const ParticleVertexStream& other);

		void Resize(uint32_t capacity);
		void Set(uint8_t* data, uint32_t count, uint32_t countOffset = 0);

		template <typename T>
		T& operator [](uint32_t index);

		template <typename T>
		const T& operator [](uint32_t index) const;

		ParticleVertexStreamType GetType() const { return m_Type; }
	private:
		const ParticleVertexStreamType m_Type;
		const uint32_t				   m_ElementSize;
		ByteBuffer					   m_Data;
	};

	template <typename T>
	T& ParticleVertexStream::operator[](uint32_t index)
	{
		XYZ_ASSERT(sizeof(T) == m_ElementSize, "");
		return *m_Data.As<T>()[index];
	}

	template <typename T>
	const T& ParticleVertexStream::operator[](uint32_t index) const
	{
		XYZ_ASSERT(sizeof(T) == m_ElementSize, "");
		return *m_Data.As<T>()[index];
	}
}
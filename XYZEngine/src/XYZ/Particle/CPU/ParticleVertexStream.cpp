#include "stdafx.h"
#include "ParticleVertexStream.h"

namespace XYZ {

	ParticleVertexStream::ParticleVertexStream(ParticleVertexStreamType type, uint32_t capacity)
		:
		m_Type(type),
		m_ElementSize(ParticleVertexStreamTypeToSize(type))
	{
		if (capacity)
			m_Data.Allocate(capacity * m_ElementSize);
	}
	ParticleVertexStream::ParticleVertexStream(ParticleVertexStream&& other) noexcept
		:
		m_Type(other.m_Type),
		m_ElementSize(other.m_ElementSize),
		m_Data(other.m_Data, other.m_Data.m_Size)
	{
		other.m_Data = nullptr;
		other.m_Data.m_Size = 0;
	}
	ParticleVertexStream::ParticleVertexStream(const ParticleVertexStream& other)
		:
		m_Type(other.m_Type),
		m_ElementSize(other.m_ElementSize)
	{
		m_Data = ByteBuffer::Copy(other.m_Data, other.m_Data.m_Size);
	}

	ParticleVertexStream::~ParticleVertexStream()
	{
		if (m_Data)
			delete[]m_Data;
	}
	void ParticleVertexStream::Set(uint8_t* data, uint32_t count, uint32_t countOffset)
	{
		XYZ_ASSERT(m_Data.m_Size > count + countOffset, "Out of range");
		m_Data.Write(data, count * m_ElementSize, countOffset);
	}
	ParticleVertexStream& ParticleVertexStream::operator=(ParticleVertexStream&& other) noexcept
	{
		XYZ_ASSERT(other.m_Type == m_Type && other.m_ElementSize == m_ElementSize, "");
		if (m_Data)
			delete []m_Data;

		m_Data = other.m_Data;
		other.m_Data = nullptr;
		other.m_Data.m_Size = 0;
	}
	ParticleVertexStream& ParticleVertexStream::operator=(const ParticleVertexStream& other)
	{
		XYZ_ASSERT(other.m_Type == m_Type && other.m_ElementSize == m_ElementSize, "");
		if (m_Data)
			delete[]m_Data;

		m_Data = ByteBuffer::Copy(other.m_Data, other.m_Data.m_Size);
	}
	void ParticleVertexStream::Resize(uint32_t capacity)
	{
		if (m_Data)
			delete[]m_Data;
		m_Data.Allocate(capacity * m_ElementSize);
	}
}
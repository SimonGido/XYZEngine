#include "stdafx.h"
#include "CustomRenderer2D.h"

#include "Renderer.h"

namespace XYZ {

	struct Renderer2DStats
	{
		uint32_t QuadDrawCalls = 0;
		uint32_t LineDrawCalls = 0;
	};

	void CustomRenderer2DData::ResetQuads()
	{
		if (!QuadBufferBase)
		{
			QuadBufferBase = new uint8_t[QuadBufferSize];
		}
		QuadIndexCount = 0;
		TextureSlotIndex = 0;
		QuadBufferWriter.Ptr = QuadBufferBase;
	}

	void CustomRenderer2DData::ResetLines()
	{
		if (!LineBufferBase)
		{
			LineBufferBase = new uint8_t[LineBufferSize];
		}
		LineIndexCount = 0;
		LineBufferWriter.Ptr = LineBufferBase;
	}

	Renderer2DStats s_Stats;
	CustomRenderer2DData CustomRenderer2D::s_Data;

	void CustomRenderer2D::Init()
	{
		s_Data.ResetQuads();
		s_Data.ResetLines();
	}
	void CustomRenderer2D::Shutdown()
	{
		delete[]s_Data.QuadBufferBase;
		delete[]s_Data.LineBufferBase;
	}
	void CustomRenderer2D::BeginScene(const CustomRenderer2DLayout& layout)
	{
		s_Data.MaxQuadIndices = (s_Data.QuadBufferSize / layout.m_QuadLayoutSize) / 4 * 6;
		s_Data.MaxLineIndices = (s_Data.LineBufferSize / layout.m_LineLayoutSize) / 4 * 6;

		s_Data.Layout = layout;
	}

	void CustomRenderer2D::SetMaterial(const Ref<Material>& material)
	{
		XYZ_ASSERT(material.Raw(), "Material can not be null");
		if (material.Raw() != s_Data.QuadMaterial.Raw())
			Flush();

		s_Data.QuadMaterial = material;
	}
	void CustomRenderer2D::SetLineShader(const Ref<Shader>& lineShader)
	{
		XYZ_ASSERT(lineShader.Raw(), "Shader can not be null");
		if (lineShader.Raw() != s_Data.LineShader.Raw())
			FlushLines();

		s_Data.LineShader = lineShader;
	}
	uint32_t CustomRenderer2D::SetTexture(const Ref<Texture>& texture)
	{
		for (uint32_t i = 0; i < s_Data.QuadMaterial->GetTextures().size(); ++i)
		{
			if (s_Data.QuadMaterial->GetTextures()[i].Raw() && s_Data.QuadMaterial->GetTextures()[i]->GetRendererID() == texture->GetRendererID())
				return i;
		}
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
		{
			if (s_Data.TextureSlots[i]->GetRendererID() == texture->GetRendererID())
				return i + s_Data.QuadMaterial->GetTextures().size();
		}

		if (s_Data.QuadMaterial->GetTextures().size() + s_Data.TextureSlotIndex >= s_Data.MaxTextures + 1)
			Flush();

		s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		return s_Data.TextureSlotIndex + s_Data.QuadMaterial->GetTextures().size() - 1;
	}
	void CustomRenderer2D::Flush()
	{
		uint32_t dataSize = s_Data.QuadBufferWriter.Ptr - s_Data.QuadBufferBase;
		if (dataSize)
		{
			XYZ_ASSERT(s_Data.QuadMaterial.Raw(), "No material set");
			
			s_Data.QuadMaterial->Bind();
			uint32_t textureSlotOffset = s_Data.QuadMaterial->GetTextures().size();
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
				s_Data.TextureSlots[i]->Bind(i + textureSlotOffset);

		
			s_Data.Layout.m_QuadVertexBuffer->Update(s_Data.QuadBufferBase, dataSize);
			s_Data.Layout.m_QuadVertexArray->Bind();

			Renderer::DrawIndexed(PrimitiveType::Triangles, s_Data.QuadIndexCount);
			s_Data.ResetQuads();
			s_Stats.QuadDrawCalls++;
		}
	}

	void CustomRenderer2D::FlushLines()
	{
		uint32_t dataSize = s_Data.LineBufferWriter.Ptr - s_Data.LineBufferBase;
		if (dataSize)
		{
			s_Data.LineShader->Bind();
			s_Data.Layout.m_LineVertexBuffer->Update(s_Data.LineBufferBase, dataSize);
			s_Data.Layout.m_LineVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Lines, s_Data.LineIndexCount);

			s_Data.ResetLines();
			s_Stats.LineDrawCalls++;
		}
	}
	void CustomRenderer2D::EndScene()
	{
		s_Stats.QuadDrawCalls = 0;
		s_Stats.LineDrawCalls = 0;
	}
	CustomRenderer2DLayout::CustomRenderer2DLayout()
		:
		m_QuadLayoutSize(0),
		m_LineLayoutSize(0)
	{
	}
	CustomRenderer2DLayout::CustomRenderer2DLayout(const BufferLayout& quadLayout, const BufferLayout& lineLayout)
	{
		SetQuadBufferLayout(quadLayout);
		SetLineBufferLayout(lineLayout);
	}
	CustomRenderer2DLayout::CustomRenderer2DLayout(const CustomRenderer2DLayout& other)
		:
		m_QuadVertexArray(other.m_QuadVertexArray),
		m_QuadVertexBuffer(other.m_QuadVertexBuffer),
		m_QuadLayoutSize(other.m_QuadLayoutSize),
		m_LineVertexArray(other.m_LineVertexArray),
		m_LineVertexBuffer(other.m_LineVertexBuffer),
		m_LineLayoutSize(other.m_LineLayoutSize)
	{
	}
	CustomRenderer2DLayout& CustomRenderer2DLayout::operator=(const CustomRenderer2DLayout& other)
	{
		m_QuadVertexArray = other.m_QuadVertexArray;
		m_QuadVertexBuffer = other.m_QuadVertexBuffer;
		m_QuadLayoutSize = other.m_QuadLayoutSize;
		m_LineVertexArray = other.m_LineVertexArray;
		m_LineVertexBuffer = other.m_LineVertexBuffer;
		m_LineLayoutSize = other.m_LineLayoutSize;
		return *this;
	}
	void CustomRenderer2DLayout::SetQuadBufferLayout(const BufferLayout& layout)
	{
		m_QuadLayoutSize = layout.GetStride();
		m_QuadVertexArray = VertexArray::Create();
		m_QuadVertexBuffer = VertexBuffer::Create(CustomRenderer2DData::QuadBufferSize);
		m_QuadVertexBuffer->SetLayout(layout);
		m_QuadVertexArray->AddVertexBuffer(m_QuadVertexBuffer);

		uint32_t maxQuadIndices = ((CustomRenderer2DData::QuadBufferSize / layout.GetStride()) / 4) * 6;
		uint32_t* quadIndices = new uint32_t[maxQuadIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < maxQuadIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, maxQuadIndices);
		m_QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;
	}
	void CustomRenderer2DLayout::SetLineBufferLayout(const BufferLayout& layout)
	{
		m_LineLayoutSize = layout.GetStride();
		m_LineVertexArray = VertexArray::Create();
		m_LineVertexBuffer = VertexBuffer::Create(CustomRenderer2DData::LineBufferSize);
		m_LineVertexBuffer->SetLayout(layout);
		m_LineVertexArray->AddVertexBuffer(m_LineVertexBuffer);

		uint32_t maxIndices = (CustomRenderer2DData::LineBufferSize / layout.GetStride()) / 2 * 6;
		uint32_t* lineIndices = new uint32_t[maxIndices];
		for (uint32_t i = 0; i < maxIndices; i++)
			lineIndices[i] = i;

		Ref<IndexBuffer> lineIndexBuffer = IndexBuffer::Create(lineIndices, maxIndices);
		m_LineVertexArray->SetIndexBuffer(lineIndexBuffer);
		delete[] lineIndices;
	}
}
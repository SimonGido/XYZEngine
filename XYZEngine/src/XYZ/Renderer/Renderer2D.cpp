#include "stdafx.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <array>

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {	

	void Renderer2D::resetQuads()
	{
		m_TextureSlotIndex = 0;
		m_QuadBuffer.Reset();
	}
	
	void Renderer2D::resetLines()
	{
		m_LineBuffer.Reset();
	}

	void Renderer2D::createRenderPass()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::Depth };
		framebufferSpec.Samples = 1;
		framebufferSpec.ClearOnLoad = false;
		framebufferSpec.ClearColor = { 0.1f, 0.5f, 0.5f, 1.0f };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(framebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		m_RenderPass = RenderPass::Create(renderPassSpec);
	}

	static uint32_t* GenerateQuadIndices(uint32_t count)
	{
		uint32_t* indices = new uint32_t[count];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < count; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;
			
			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		return indices;
	}
	
	static uint32_t* GenerateLineIndices(uint32_t count)
	{
		uint32_t* indices = new uint32_t[count];
		for (uint32_t i = 0; i < count; i++)
		{
			indices[i] = i;
		}
		return indices;
	}

	Renderer2D::Renderer2D(Renderer2DSpecification specification)
		:
		m_Specification(specification)
	{
		if (specification.SwapChainTarget)
			m_RenderCommandBuffer = Renderer::GetAPIContext()->GetRenderCommandBuffer();
		else
			m_RenderCommandBuffer = RenderCommandBuffer::Create(0, "Renderer2D");
		
		createRenderPass();

		uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBCamera), 0, 0);

		auto shaderLibrary	 = Renderer::GetShaderLibrary();
		
		m_QuadMaterial = Material::Create(shaderLibrary->Get("DefaultShader"));
		m_LineMaterial = Material::Create(shaderLibrary->Get("LineShader"));
		m_CircleMaterial = Material::Create(shaderLibrary->Get("Circle"));

		m_WhiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1, {});
		uint32_t whiteTextureData = 0xffffffff;
		m_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		m_TextureSlots[0] = m_WhiteTexture;

		uint32_t* quadIndices = GenerateQuadIndices(sc_MaxIndices);
		m_QuadBuffer.Init(m_RenderPass, m_QuadMaterial->GetShader(), sc_MaxVertices, quadIndices, sc_MaxIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float4, "a_Color" },
				{1, XYZ::ShaderDataType::Float3, "a_Position" },
				{2, XYZ::ShaderDataType::Float2, "a_TexCoord" },
				{3, XYZ::ShaderDataType::Float,  "a_TextureID" },
				{4, XYZ::ShaderDataType::Float,  "a_TilingFactor" }
		});

		uint32_t* lineIndices = GenerateLineIndices(sc_MaxLineIndices);
		m_LineBuffer.Init(m_RenderPass, m_LineMaterial->GetShader(), sc_MaxLineVertices, lineIndices, sc_MaxLineIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float3, "a_Position" },
				{1, XYZ::ShaderDataType::Float4, "a_Color" },
		});

		m_CircleBuffer.Init(m_RenderPass, m_CircleMaterial->GetShader(), sc_MaxVertices, quadIndices, sc_MaxIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float3, "a_WorldPosition" },
				{1, XYZ::ShaderDataType::Float,  "a_Thickness" },
				{2, XYZ::ShaderDataType::Float2, "a_LocalPosition" },
				{3, XYZ::ShaderDataType::Float4, "a_Color" }
			});

		delete[]quadIndices;
		delete[]lineIndices;
	}

	Renderer2D::~Renderer2D()
	{
	}


	void Renderer2D::BeginScene()
	{
		m_Stats.DrawCalls = 0;
		m_Stats.LineDrawCalls = 0;
		m_Stats.CollisionDrawCalls = 0;
		m_Stats.FilledCircleDrawCalls = 0;
	}

	void Renderer2D::SetQuadMaterial(const Ref<Material>& material)
	{
		m_QuadMaterial = material;
		setMaterial(m_QuadBuffer.Pipeline, material);
	}

	void Renderer2D::SetLineMaterial(const Ref<Material>& material)
	{
		m_LineMaterial = material;
		setMaterial(m_LineBuffer.Pipeline, material);
	}

	void Renderer2D::SetCircleMaterial(const Ref<Material>& material)
	{
		m_CircleMaterial = material;
		setMaterial(m_CircleBuffer.Pipeline, material);
	}

	void Renderer2D::SetTargetRenderPass(const Ref<RenderPass>& renderPass)
	{
		m_RenderPass = renderPass;
		updateRenderPass(m_QuadBuffer.Pipeline);
		updateRenderPass(m_LineBuffer.Pipeline);
		updateRenderPass(m_CircleBuffer.Pipeline);
	}

	Ref<RenderPass> Renderer2D::GetTargetRenderPass()
	{
		return m_RenderPass;
	}
	
	void Renderer2D::SubmitCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color)
	{
		if (m_LineBuffer.IndexCount + (sides * 3) >= sc_MaxLineIndices)
			XYZ_ASSERT(false, "")

		const int step = 360 / sides;
		for (int a = step; a < 360 + step; a += step)
		{
			const float before = glm::radians((float)(a - step));
			const float heading = glm::radians((float)a);
			
			m_LineBuffer.BufferPtr->Position = glm::vec3(pos.x + std::cos(before) * radius, pos.y + std::sin(before) * radius, pos.z);
			m_LineBuffer.BufferPtr->Color = color;
			m_LineBuffer.BufferPtr++;
			m_LineBuffer.BufferPtr->Position = glm::vec3(pos.x + std::cos(heading) * radius, pos.y + std::sin(heading) * radius, pos.z);
			m_LineBuffer.BufferPtr->Color = color;
			m_LineBuffer.BufferPtr++;
			m_LineBuffer.IndexCount += 2;
		}
	}

	void Renderer2D::SubmitFilledCircle(const glm::vec3& pos, float radius, float thickness, const glm::vec4& color)
	{
		if (m_CircleBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), { radius * 2.0f, radius * 2.0f, 1.0f });

		for (int i = 0; i < 4; i++)
		{
			m_CircleBuffer.BufferPtr->WorldPosition = transform * sc_QuadVertexPositions[i];
			m_CircleBuffer.BufferPtr->Thickness = thickness;
			m_CircleBuffer.BufferPtr->LocalPosition = sc_QuadVertexPositions[i] * 2.0f;
			m_CircleBuffer.BufferPtr->Color = color;
			m_CircleBuffer.BufferPtr++;		
		}
		m_CircleBuffer.IndexCount += 6;
	}


	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			m_QuadBuffer.BufferPtr->Position = transform * sc_QuadVertexPositions[i];
			m_QuadBuffer.BufferPtr->Color = color;
			m_QuadBuffer.BufferPtr->TexCoord = glm::vec2(0);
			m_QuadBuffer.BufferPtr->TextureID = 0.0f;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		const glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			m_QuadBuffer.BufferPtr->Position = transform * sc_QuadVertexPositions[i];
			m_QuadBuffer.BufferPtr->Color = color;
			m_QuadBuffer.BufferPtr->TexCoord = texCoords[i];
			m_QuadBuffer.BufferPtr->TextureID = (float)textureID;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		const glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x, texCoord.y},
			{texCoord.z, texCoord.y},
			{texCoord.z, texCoord.w},
			{texCoord.x, texCoord.w}
		};
		const glm::vec3 vertices[quadVertexCount] = {
			{  position.x - size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f},
			{  position.x - size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			m_QuadBuffer.BufferPtr->Position = vertices[i];
			m_QuadBuffer.BufferPtr->Color = color;
			m_QuadBuffer.BufferPtr->TexCoord = texCoords[i];
			m_QuadBuffer.BufferPtr->TextureID = (float)textureID;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		const glm::vec3 vertices[quadVertexCount] = {
			{  position.x - size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f},
			{  position.x - size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			m_QuadBuffer.BufferPtr->Position = vertices[i];
			m_QuadBuffer.BufferPtr->Color = color;
			m_QuadBuffer.BufferPtr->TexCoord = glm::vec2(0);
			m_QuadBuffer.BufferPtr->TextureID = 0.0f;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (m_LineBuffer.IndexCount >= sc_MaxLineIndices)
			XYZ_ASSERT(false, "")

		m_LineBuffer.BufferPtr->Position = p0;
		m_LineBuffer.BufferPtr->Color = color;
		m_LineBuffer.BufferPtr++;
		m_LineBuffer.BufferPtr->Position = p1;
		m_LineBuffer.BufferPtr->Color = color;
		m_LineBuffer.BufferPtr++;

		m_LineBuffer.IndexCount += 2;
	}

	void Renderer2D::SubmitLineQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		const glm::vec3 p[4] = {
			position,
			position + glm::vec3(size.x, 0.0f, 0.0f),
			position + glm::vec3(size.x, size.y, 0.0f),
			position + glm::vec3(0.0f,size.y, 0.0f)
		};
		SubmitLine(p[0], p[1], color);
		SubmitLine(p[1], p[2], color);
		SubmitLine(p[2], p[3], color);
		SubmitLine(p[3], p[0], color);
	}

	void Renderer2D::SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "")

		const glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x, texCoord.y},
			{texCoord.z, texCoord.y},
			{texCoord.z, texCoord.w},
			{texCoord.x, texCoord.w}
		};

		const glm::vec3 vertices[quadVertexCount] = {
			{  position.x ,			 position.y, 0.0f},
			{  position.x + size.x,  position.y, 0.0f},
			{  position.x + size.x,  position.y + size.y, 0.0f},
			{  position.x,			 position.y + size.y, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			m_QuadBuffer.BufferPtr->Position = vertices[i];
			m_QuadBuffer.BufferPtr->Color = color;
			m_QuadBuffer.BufferPtr->TexCoord = texCoords[i];
			m_QuadBuffer.BufferPtr->TextureID = (float)textureID;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}


	void Renderer2D::EndScene()
	{
		m_RenderCommandBuffer->Begin();
		Renderer::BeginRenderPass(m_RenderCommandBuffer, m_RenderPass, true);

		flush();
		flushLines();
		flushFilledCircles();

		Renderer::EndRenderPass(m_RenderPass);
		m_RenderCommandBuffer->End();
		m_RenderCommandBuffer->Submit();
	}

	void Renderer2D::flush()
	{
		const uint32_t dataSize = (uint8_t*)m_QuadBuffer.BufferPtr - (uint8_t*)m_QuadBuffer.BufferBase;
		if (dataSize)
		{
			XYZ_ASSERT(m_QuadMaterial.Raw(), "No material set");
						
			m_QuadBuffer.VertexBuffer->Update(m_QuadBuffer.BufferBase, dataSize);

			Renderer::DrawIndexed(PrimitiveType::Triangles, m_QuadBuffer.IndexCount);
			Renderer::BindPipeline(m_RenderCommandBuffer, m_QuadBuffer.Pipeline, m_UniformBufferSet, m_QuadMaterial);
			Renderer::RenderGeometry(m_RenderCommandBuffer, m_QuadBuffer.Pipeline, m_QuadMaterial, m_QuadBuffer.VertexBuffer, m_QuadBuffer.IndexBuffer, glm::mat4(1.0f));
			m_Stats.DrawCalls++;
			m_QuadBuffer.Reset();
		}	

		
	}
	void Renderer2D::flushLines()
	{
		const uint32_t dataSize = (uint8_t*)m_LineBuffer.BufferPtr - (uint8_t*)m_LineBuffer.BufferBase;
		if (dataSize)
		{
			m_LineBuffer.VertexBuffer->Update(m_LineBuffer.BufferBase, dataSize);

			Renderer::DrawIndexed(PrimitiveType::Lines, m_LineBuffer.IndexCount);

			m_Stats.LineDrawCalls++;
			m_LineBuffer.Reset();;
		}	
	}


	void Renderer2D::flushFilledCircles()
	{
		const uint32_t dataSize = (uint8_t*)m_CircleBuffer.BufferPtr - (uint8_t*)m_CircleBuffer.BufferBase;
		if (dataSize)
		{
			m_CircleBuffer.VertexBuffer->Update(m_CircleBuffer.BufferBase, dataSize);

			Renderer::DrawIndexed(PrimitiveType::Triangles, m_CircleBuffer.IndexCount);

			m_Stats.FilledCircleDrawCalls++;
			m_CircleBuffer.Reset();
		}
	}

	void Renderer2D::updateRenderPass(Ref<Pipeline>& pipeline) const
	{
		auto spec = pipeline->GetSpecification();
		spec.RenderPass = m_RenderPass;
		pipeline = Pipeline::Create(spec);
	}

	void Renderer2D::setMaterial(Ref<Pipeline>& pipeline, const Ref<Material>& material)
	{
		auto spec = pipeline->GetSpecification();
		spec.Shader = material->GetShader();
		pipeline = Pipeline::Create(spec);
	}

	const Renderer2DStats& Renderer2D::GetStats()
	{
		return m_Stats;
	}
}
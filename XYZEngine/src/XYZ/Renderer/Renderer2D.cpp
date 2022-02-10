#include "stdafx.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace XYZ {	

	

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

	Renderer2D::Renderer2D(const Ref<RenderCommandBuffer>& commandBuffer)
		:
		m_RenderCommandBuffer(commandBuffer)
	{
		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			
		createRenderPass();	
		auto shaderLibrary = Renderer::GetShaderLibrary();
		
		const auto& defaultResources = Renderer::GetDefaultResources();

		m_QuadMaterial = defaultResources.DefaultQuadMaterial;
		m_LineMaterial = defaultResources.DefaultLineMaterial;
		m_CircleMaterial = defaultResources.DefaultCircleMaterial;

		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBCamera), 0, 0);
		m_UniformBufferSet->CreateDescriptors(m_QuadMaterial->GetShader());
		m_UniformBufferSet->CreateDescriptors(m_LineMaterial->GetShader());
		m_UniformBufferSet->CreateDescriptors(m_CircleMaterial->GetShader());
	

		createDefaultPipelineBuckets();
	}

	Renderer2D::Renderer2D(const Ref<RenderCommandBuffer>& commandBuffer, 
		const Ref<Material>& quadMaterial, 
		const Ref<Material>& lineMaterial, 
		const Ref<Material>& circleMaterial,
		const Ref<RenderPass>& renderPass
	)
		:
		m_RenderCommandBuffer(commandBuffer),
		m_QuadMaterial(quadMaterial),
		m_LineMaterial(lineMaterial),
		m_CircleMaterial(circleMaterial)
	{
		if (!renderPass.Raw())
			createRenderPass();
		else
			m_RenderPass = renderPass;

		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBCamera), 0, 0);
		m_UniformBufferSet->CreateDescriptors(m_QuadMaterial->GetShader());
		m_UniformBufferSet->CreateDescriptors(m_LineMaterial->GetShader());
		m_UniformBufferSet->CreateDescriptors(m_CircleMaterial->GetShader());


		createDefaultPipelineBuckets();
	}

	Renderer2D::~Renderer2D()
	{
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, bool clear)
	{
		m_Stats.DrawCalls = 0;
		m_Stats.LineDrawCalls = 0;
		m_Stats.CollisionDrawCalls = 0;
		m_Stats.FilledCircleDrawCalls = 0;

		const uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
		m_UniformBufferSet->Get(0, 0, currentFrame)->Update(&viewProjectionMatrix, sizeof(UBCamera), 0);
		m_ViewMatrix = viewMatrix;

		Renderer::BeginRenderPass(m_RenderCommandBuffer, m_RenderPass, clear);

	}


	void Renderer2D::SetQuadMaterial(const Ref<Material>& material)
	{
		if (m_QuadMaterial.Raw() != material.Raw())
		{
			m_QuadMaterial = material;
			m_QuadBuffer.Pipeline = setMaterial(m_QuadPipelines, m_QuadBuffer.Pipeline, material);
		}
	}

	void Renderer2D::SetLineMaterial(const Ref<Material>& material)
	{
		if (m_LineMaterial.Raw() != material.Raw())
		{
			m_LineMaterial = material;
			m_LineBuffer.Pipeline = setMaterial(m_LinePipelines, m_LineBuffer.Pipeline, material);
		}
	}

	void Renderer2D::SetCircleMaterial(const Ref<Material>& material)
	{
		if (m_CircleMaterial.Raw() != material.Raw())
		{
			m_CircleMaterial = material;
			m_CircleBuffer.Pipeline = setMaterial(m_CirclePipelines, m_CircleBuffer.Pipeline, material);
		}
	}

	void Renderer2D::SetTargetRenderPass(const Ref<RenderPass>& renderPass)
	{
		if (m_RenderPass.Raw() != renderPass.Raw())
		{
			m_RenderPass = renderPass;
			updateRenderPass(m_QuadBuffer.Pipeline);
			updateRenderPass(m_LineBuffer.Pipeline);
			updateRenderPass(m_CircleBuffer.Pipeline);
		}
	}

	Ref<RenderPass> Renderer2D::GetTargetRenderPass() const
	{
		return m_RenderPass;
	}
	
	void Renderer2D::SubmitCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color)
	{
		if (m_LineBuffer.IndexCount + (sides * 3) >= sc_MaxLineIndices)
			XYZ_ASSERT(false, "");

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

	void Renderer2D::SubmitFilledCircle(const glm::vec3& pos, const glm::vec2& size, float thickness, const glm::vec4& color)
	{
		if (m_CircleBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

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


	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureIndex, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

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
			m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

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



	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureIndex, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

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
			m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}


	void Renderer2D::SubmitQuadBillboard(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		glm::vec3 camRightWS = { m_ViewMatrix[0][0], m_ViewMatrix[1][0], m_ViewMatrix[2][0] };
		glm::vec3 camUpWS = { m_ViewMatrix[0][1], m_ViewMatrix[1][1], m_ViewMatrix[2][1] };

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * (sc_QuadVertexPositions[0].x) * size.x + camUpWS * sc_QuadVertexPositions[0].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = { 0.0f, 0.0f };
		m_QuadBuffer.BufferPtr->TextureID = textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[1].x * size.x + camUpWS * sc_QuadVertexPositions[1].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = { 1.0f, 0.0f };
		m_QuadBuffer.BufferPtr->TextureID = textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[2].x * size.x + camUpWS * sc_QuadVertexPositions[2].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = { 1.0f, 1.0f };
		m_QuadBuffer.BufferPtr->TextureID = textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[3].x * size.x + camUpWS * sc_QuadVertexPositions[3].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = { 0.0f, 1.0f };
		m_QuadBuffer.BufferPtr->TextureID = textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.IndexCount += 6;
	}
	void Renderer2D::SubmitQuadBillboard(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureIndex, const glm::vec4& color, float tilingFactor)
	{
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

		const glm::vec2 texCoords[4] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};

		glm::vec3 camRightWS = { m_ViewMatrix[0][0], m_ViewMatrix[1][0], m_ViewMatrix[2][0] };
		glm::vec3 camUpWS = { m_ViewMatrix[0][1], m_ViewMatrix[1][1], m_ViewMatrix[2][1] };

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * (sc_QuadVertexPositions[0].x) * size.x + camUpWS * sc_QuadVertexPositions[0].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = texCoords[0];
		m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[1].x * size.x + camUpWS * sc_QuadVertexPositions[1].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = texCoords[1];
		m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[2].x * size.x + camUpWS * sc_QuadVertexPositions[2].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = texCoords[2];
		m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.BufferPtr->Position = position + camRightWS * sc_QuadVertexPositions[3].x * size.x + camUpWS * sc_QuadVertexPositions[3].y * size.y;
		m_QuadBuffer.BufferPtr->Color = color;
		m_QuadBuffer.BufferPtr->TexCoord = texCoords[3];
		m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
		m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
		m_QuadBuffer.BufferPtr++;

		m_QuadBuffer.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		if (m_QuadBuffer.IndexCount >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

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

	void Renderer2D::SubmitRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
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

	void Renderer2D::SubmitRect(const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * sc_QuadVertexPositions[i];

		SubmitLine(lineVertices[0], lineVertices[1], color);
		SubmitLine(lineVertices[1], lineVertices[2], color);
		SubmitLine(lineVertices[2], lineVertices[3], color);
		SubmitLine(lineVertices[3], lineVertices[0], color);
	}


	void Renderer2D::SubmitAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color)
	{
		glm::vec3 topFrontLeft = { min.x, max.y, min.z };
		glm::vec3 topFrontRight = { max.x, max.y, min.z };
		glm::vec3 bottomFrontLeft = { min.x, min.y, min.z };
		glm::vec3 bottomFrontRight = { max.x, min.y, min.z };


		glm::vec3 topBackLeft = { min.x, max.y, max.z };
		glm::vec3 topBackRight = { max.x, max.y, max.z };
		glm::vec3 bottomBackLeft = { min.x, min.y, max.z };
		glm::vec3 bottomBackRight = { max.x, min.y, max.z };


		// Front
		SubmitLine(topFrontLeft, topFrontRight, color);
		SubmitLine(topFrontRight, bottomFrontRight, color);
		SubmitLine(bottomFrontRight, bottomFrontLeft, color);
		SubmitLine(bottomFrontLeft, topFrontLeft, color);


		//Back
		SubmitLine(topBackLeft, topBackRight, color);
		SubmitLine(topBackRight, bottomBackRight, color);
		SubmitLine(bottomBackRight, bottomBackLeft, color);
		SubmitLine(bottomBackLeft, topBackLeft, color);

		// Sides
		SubmitLine(topFrontLeft, topBackLeft, color);
		SubmitLine(topFrontRight, topBackRight, color);
		SubmitLine(bottomFrontLeft,  bottomBackLeft, color);
		SubmitLine(bottomFrontRight, bottomBackRight, color);
	}


	void Renderer2D::SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureIndex, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (m_QuadBuffer.IndexCount + 6 >= sc_MaxIndices)
			XYZ_ASSERT(false, "");

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
			m_QuadBuffer.BufferPtr->TextureID = (float)textureIndex;
			m_QuadBuffer.BufferPtr->TilingFactor = tilingFactor;
			m_QuadBuffer.BufferPtr++;
		}
		m_QuadBuffer.IndexCount += 6;
	}


	void Renderer2D::Flush()
	{
		flush();
		flushLines();
		flushFilledCircles();
	}

	void Renderer2D::EndScene()
	{	
		Flush();
		m_QuadBuffer.Offset = 0;
		Renderer::EndRenderPass(m_RenderCommandBuffer);
	}
	void Renderer2D::resetQuads()
	{
		m_QuadBuffer.Reset();
	}

	void Renderer2D::resetLines()
	{
		m_LineBuffer.Reset();
	}

	void Renderer2D::createRenderPass()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::RGBA32F)
		};
		framebufferSpec.Samples = 1;
		framebufferSpec.ClearOnLoad = false;
		framebufferSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(framebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		m_RenderPass = RenderPass::Create(renderPassSpec);
	}

	void Renderer2D::createDefaultPipelineBuckets()
	{
		uint32_t* quadIndices = GenerateQuadIndices(sc_MaxIndices);
		
		m_QuadBuffer.Init(m_RenderPass, m_QuadMaterial->GetShader(), sc_MaxVertices, quadIndices, sc_MaxIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float4, "a_Color" },
				{1, XYZ::ShaderDataType::Float3, "a_Position" },
				{2, XYZ::ShaderDataType::Float2, "a_TexCoord" },
				{3, XYZ::ShaderDataType::Float,  "a_TextureID" },
				{4, XYZ::ShaderDataType::Float,  "a_TilingFactor" }
			});
		const size_t quadShaderHash = m_QuadMaterial->GetShader()->GetHash();
		m_QuadPipelines.emplace(quadShaderHash, m_QuadBuffer.Pipeline);

		uint32_t* lineIndices = GenerateLineIndices(sc_MaxLineIndices);
		m_LineBuffer.Init(m_RenderPass, m_LineMaterial->GetShader(), sc_MaxLineVertices, lineIndices, sc_MaxLineIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float4, "a_Color" },
				{1, XYZ::ShaderDataType::Float3, "a_Position" },
			}, PrimitiveTopology::Lines);
		const size_t lineShaderHash = m_LineMaterial->GetShader()->GetHash();
		m_LinePipelines.emplace(lineShaderHash, m_LineBuffer.Pipeline);


		m_CircleBuffer.Init(m_RenderPass, m_CircleMaterial->GetShader(), sc_MaxVertices, quadIndices, sc_MaxIndices, BufferLayout{
				{0, XYZ::ShaderDataType::Float3, "a_WorldPosition" },
				{1, XYZ::ShaderDataType::Float,  "a_Thickness" },
				{2, XYZ::ShaderDataType::Float2, "a_LocalPosition" },
				{3, XYZ::ShaderDataType::Float4, "a_Color" }
			});
		const size_t circleShaderHash = m_CircleMaterial->GetShader()->GetHash();
		m_CirclePipelines.emplace(circleShaderHash, m_CircleBuffer.Pipeline);

		delete[]quadIndices;
		delete[]lineIndices;
	}
	void Renderer2D::flush()
	{
		const uint32_t dataSize = (uint8_t*)m_QuadBuffer.BufferPtr - (uint8_t*)m_QuadBuffer.BufferBase;
		if (dataSize)
		{
			XYZ_ASSERT(m_QuadMaterial.Raw(), "No material set");
			m_QuadBuffer.VertexBuffer[m_QuadBuffer.Offset]->Update(m_QuadBuffer.BufferBase, dataSize, m_QuadBuffer.Offset);

			Renderer::BindPipeline(m_RenderCommandBuffer, m_QuadBuffer.Pipeline, m_UniformBufferSet, nullptr, m_QuadMaterial);
			Renderer::RenderGeometry(m_RenderCommandBuffer, m_QuadBuffer.Pipeline, m_QuadMaterial, m_QuadBuffer.VertexBuffer[m_QuadBuffer.Offset], m_QuadBuffer.IndexBuffer, glm::mat4(1.0f), m_QuadBuffer.IndexCount);
			m_Stats.DrawCalls++;
			m_QuadBuffer.Reset();
			m_QuadBuffer.Offset++;
		}		
	}
	void Renderer2D::flushLines()
	{
		const uint32_t dataSize = (uint8_t*)m_LineBuffer.BufferPtr - (uint8_t*)m_LineBuffer.BufferBase;
		if (dataSize)
		{
			//m_LineBuffer.VertexBuffer->Update(m_LineBuffer.BufferBase, dataSize);
			//
			//Renderer::BindPipeline(m_RenderCommandBuffer, m_LineBuffer.Pipeline, m_UniformBufferSet, nullptr, m_LineMaterial);
			//Renderer::RenderGeometry(m_RenderCommandBuffer, m_LineBuffer.Pipeline, m_LineMaterial, m_LineBuffer.VertexBuffer, m_LineBuffer.IndexBuffer, glm::mat4(1.0f), m_LineBuffer.IndexCount);

			m_Stats.LineDrawCalls++;
			m_LineBuffer.Reset();;
		}	
	}


	void Renderer2D::flushFilledCircles()
	{
		const uint32_t dataSize = (uint8_t*)m_CircleBuffer.BufferPtr - (uint8_t*)m_CircleBuffer.BufferBase;
		if (dataSize)
		{
			//m_CircleBuffer.VertexBuffer->Update(m_CircleBuffer.BufferBase, dataSize);
			//
			//Renderer::BindPipeline(m_RenderCommandBuffer, m_CircleBuffer.Pipeline, m_UniformBufferSet, nullptr, m_CircleMaterial);
			//Renderer::RenderGeometry(m_RenderCommandBuffer, m_CircleBuffer.Pipeline, m_CircleMaterial, m_CircleBuffer.VertexBuffer, m_CircleBuffer.IndexBuffer, glm::mat4(1.0f), m_CircleBuffer.IndexCount);

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

	Ref<Pipeline> Renderer2D::setMaterial(std::map<size_t, Ref<Pipeline>>& pipelines, const Ref<Pipeline>& current, const Ref<Material>& material)
	{
		const size_t shaderHash = material->GetShader()->GetHash();
		auto it = pipelines.find(shaderHash);
		if (it != pipelines.end())
			return it->second;
		
		auto spec = current->GetSpecification();
		spec.Shader = material->GetShader();
		if (!m_UniformBufferSet->HasDescriptors(spec.Shader->GetHash()))
			m_UniformBufferSet->CreateDescriptors(spec.Shader);
		return Pipeline::Create(spec);
	}


	const Renderer2DStats& Renderer2D::GetStats()
	{
		return m_Stats;
	}
}
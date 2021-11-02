#include "stdafx.h"
#include "Renderer.h"

#include "CustomRenderer2D.h"
#include "Renderer2D.h"
#include "SceneRenderer.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"

#include <GL/glew.h>

namespace XYZ {
	
	
	struct RendererData
	{
		std::shared_ptr<ThreadPass<RenderCommandQueue>> m_CommandQueue;
		ThreadPool									    m_Pool;
		Ref<ShaderLibrary>								m_ShaderLibrary;
		Ref<RenderPass>									m_ActiveRenderPass;
		Ref<VertexArray>								m_FullscreenQuadVertexArray;
		Ref<VertexBuffer>								m_FullscreenQuadVertexBuffer;
		Ref<IndexBuffer>								m_FullscreenQuadIndexBuffer;
														
		std::future<bool>							    m_RenderThreadFinished;
		RendererStats									m_Stats;
		std::mutex										m_QueueLock;
	};

	static RendererData s_Data;

	static void SetupFullscreenQuad()
	{
		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		BufferLayout layout = {
			{ 0, ShaderDataComponent::Float3, "a_Position" },
			{ 1, ShaderDataComponent::Float2, "a_TexCoord" }
		};
		s_Data.m_FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		s_Data.m_FullscreenQuadVertexBuffer->SetLayout(layout);
		s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(s_Data.m_FullscreenQuadVertexBuffer);

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_Data.m_FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6);
		s_Data.m_FullscreenQuadVertexArray->SetIndexBuffer(s_Data.m_FullscreenQuadIndexBuffer);
	}

	void Renderer::Init()
	{
		s_Data.m_Pool.PushThread();
		s_Data.m_CommandQueue = std::make_shared<ThreadPass<RenderCommandQueue>>();
		Renderer::Submit([=]() {
			RendererAPI::Init();
		});
		

		SetupFullscreenQuad();

		s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/RendererCore/CompositeShader.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/RendererCore/LightShader.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/RendererCore/Bloom.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/RendererCore/Circle.glsl");

		s_Data.m_ShaderLibrary->Load("Assets/Shaders/DefaultLitShader.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/DefaultShader.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/LineShader.glsl");
		s_Data.m_ShaderLibrary->Load("Assets/Shaders/MousePicker.glsl");

		s_Data.m_ShaderLibrary->Load("Assets/Shaders/Particle/ParticleShaderCPU.glsl");
	}

	void Renderer::Shutdown()
	{
		s_Data.m_FullscreenQuadVertexArray.Reset();
		s_Data.m_FullscreenQuadVertexBuffer.Reset();
		s_Data.m_FullscreenQuadIndexBuffer.Reset();
		s_Data.m_ShaderLibrary.Reset();
		auto queue = s_Data.m_CommandQueue;
		queue->Swap();
	}

	void Renderer::Lock()
	{
		s_Data.m_QueueLock.lock();
	}

	void Renderer::Unlock()
	{
		s_Data.m_QueueLock.unlock();
	}

	void Renderer::Clear()
	{
		Renderer::Submit([=]() {
			RendererAPI::Clear();
		});
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetClearColor(color);
		});
	}

	void Renderer::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetViewport(x, y, width, height);
		});
	}

	void Renderer::SetLineThickness(float thickness)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetLineThickness(thickness);
			});
	}

	void Renderer::SetPointSize(float size)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetPointSize(size);
			});
	}

	void Renderer::SetDepthTest(bool val)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetDepth(val);
			});
	}

	void Renderer::DrawArrays(PrimitiveType type, uint32_t count)
	{
		s_Data.m_Stats.DrawArraysCount++;
		Renderer::Submit([=]() {
			RendererAPI::DrawArrays(type, count);
			});
	}

	void Renderer::DrawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t queueType)
	{
		s_Data.m_Stats.DrawIndexedCount++;
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(type, indexCount);
		}, queueType);
	}

	void Renderer::DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset, uint32_t queueType)
	{
		s_Data.m_Stats.DrawInstancedCount++;
		
		Renderer::Submit([=]() {
			RendererAPI::DrawInstanced(type, indexCount, instanceCount, offset);
		}, queueType);
	}

	void Renderer::DrawElementsIndirect(void* indirect)
	{
		s_Data.m_Stats.DrawIndirectCount++;
		Renderer::Submit([=]() {
			RendererAPI::DrawInstancedIndirect(indirect);
		});
	}

	void Renderer::SubmitFullscreenQuad()
	{
		s_Data.m_Stats.DrawFullscreenCount++;
		s_Data.m_FullscreenQuadVertexArray->Bind();
		Renderer::DrawIndexed(PrimitiveType::Triangles, 6);
	}


	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear)
	{
		XYZ_ASSERT(renderPass.Raw(), "Render pass can not be null");
		s_Data.m_ActiveRenderPass = renderPass;
		const Ref<Framebuffer>& frameBuffer = s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer;
		if (!frameBuffer->GetSpecification().SwapChainTarget)
			s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Bind();

		if (clear)
		{
			renderPass->GetSpecification().TargetFramebuffer->Clear();
		}
	}

	void Renderer::EndRenderPass()
	{
		XYZ_ASSERT(s_Data.m_ActiveRenderPass.Raw(), "No active render pass! Have you called Renderer::EndRenderPass twice?");
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_Data.m_ActiveRenderPass = nullptr;
	}

	void Renderer::BlockRenderThread()
	{
		s_Data.m_RenderThreadFinished.wait();
	}

	ThreadPool& Renderer::GetPool()
	{
		return s_Data.m_Pool;
	}

	const RendererStats& Renderer::GetStats()
	{
		return s_Data.m_Stats;
	}

	void Renderer::WaitAndRender()
	{
		s_Data.m_Stats.Reset();

		auto queue = s_Data.m_CommandQueue;
		queue->Swap();
		#ifdef RENDER_THREAD_ENABLED
		s_Data.m_RenderThreadFinished = s_Data.m_Pool.PushJob<bool>([queue]() {
			XYZ_PROFILE_FUNC("Renderer::WaitAndRender Job");
			auto val = queue->Read();
			val->Execute();
		
			GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			auto value = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
			glDeleteSync(fence);
			return true;
		});
		#else
		{
			auto val = queue->Read();
			val->Execute();
		}
		#endif
	}
	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.m_ShaderLibrary;
	}
	ScopedLock<RenderCommandQueue> Renderer::getRenderCommandQueue(uint8_t type)
	{
		return s_Data.m_CommandQueue->Write();
	}
	RendererStats& Renderer::getStats()
	{
		return s_Data.m_Stats;
	}
	RendererStats::RendererStats()
		:
		DrawArraysCount(0), DrawIndexedCount(0), DrawInstancedCount(0), DrawFullscreenCount(0), DrawIndirectCount(0), CommandsCount(0)
	{
	}
	void RendererStats::Reset()
	{
		DrawArraysCount = 0;
		DrawIndexedCount = 0;
		DrawInstancedCount = 0;
		DrawFullscreenCount = 0;
		DrawIndirectCount = 0;
		CommandsCount = 0;
	}
}
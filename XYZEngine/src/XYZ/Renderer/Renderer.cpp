#include "stdafx.h"
#include "Renderer.h"

#include "CustomRenderer2D.h"
#include "Renderer2D.h"
#include "SceneRenderer.h"
#include "Fence.h"
#include "RendererQueueData.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"


#include "XYZ/API/OpenGL/OpenGLRendererAPI.h"
#include "XYZ/API/Vulkan/VulkanRendererAPI.h"

namespace XYZ {


	struct ShaderDependencies
	{
		void Clear()
		{
			MaterialDependencies.clear();
			PipelineDependencies.clear();
		}
		std::vector<Ref<Material>> MaterialDependencies;
		std::vector<Ref<Pipeline>> PipelineDependencies;
	};

	struct ShaderDependencyMap
	{
		void OnReload(size_t hash)
		{
			auto it = m_Dependencies.find(hash);
			if (it != m_Dependencies.end())
			{
				for (auto& material : it->second.MaterialDependencies)
					material->Invalidate();
				for (auto& pipeline : it->second.PipelineDependencies)
					pipeline->Invalidate();
			}
		}
		void Register(size_t hash, const Ref<Material>& material)
		{
			m_Dependencies[hash].MaterialDependencies.push_back(material);
		}
		void Register(size_t hash, const Ref<Pipeline>& pipeline)
		{
			m_Dependencies[hash].PipelineDependencies.push_back(pipeline);
		}
		void RemoveDependency(size_t hash)
		{
			auto it = m_Dependencies.find(hash);
			if (it != m_Dependencies.end())
				it->second.Clear();
		}

		void Clear()
		{
			for (auto&& [hash, dep] : m_Dependencies)
				dep.Clear();
		}
	private:
		std::unordered_map<size_t, ShaderDependencies> m_Dependencies;
	};

	

	struct RendererData
	{
		RendererQueueData			   QueueData;
		Ref<APIContext>				   APIContext;
		Ref<ShaderLibrary>			   ShaderLibrary;
		Ref<VertexBuffer>			   FullscreenQuadVertexBuffer;
		Ref<IndexBuffer>			   FullscreenQuadIndexBuffer;

		
		
		RendererStats				   Stats;
		RendererConfiguration		   Configuration;
		RendererResources			   Resources;
		ShaderDependencyMap			   ShaderDependencies;
	};

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	static RendererData s_Data;
	static RendererAPI* s_RendererAPI = nullptr;

	static RendererAPI* CreateRendererAPI()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: return new OpenGLRendererAPI();
		case RendererAPI::API::Vulkan: return new VulkanRendererAPI();
		}
		XYZ_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	static void SetupFullscreenQuad()
	{
		const float x = -1;
		const float y = -1;
		const float width = 2, height = 2;
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

		const BufferLayout layout = {
			{ 0, ShaderDataType::Float3, "a_Position" },
			{ 1, ShaderDataType::Float2, "a_TexCoord" }
		};
		s_Data.FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		s_Data.FullscreenQuadVertexBuffer->SetLayout(layout);
	
		const uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_Data.FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6);
	}

	void Renderer::Init(const RendererConfiguration& config)
	{
		s_Data.Configuration = config;
	
		s_Data.APIContext = APIContext::Create();
		s_RendererAPI = CreateRendererAPI();

		s_Data.QueueData.Init(config.FramesInFlight);
		s_RendererAPI->Init();
	}

	void Renderer::InitResources()
	{
		SetupFullscreenQuad();

		s_Data.ShaderLibrary = Ref<ShaderLibrary>::Create();
		s_Data.ShaderLibrary->Load("Assets/Shaders/RendererCore/CompositeShader.glsl");
		s_Data.ShaderLibrary->Load("Assets/Shaders/RendererCore/LightShader.glsl");
		s_Data.ShaderLibrary->Load("Assets/Shaders/RendererCore/Bloom.glsl");
		s_Data.ShaderLibrary->Load("Assets/Shaders/RendererCore/Circle.glsl");
		
		s_Data.ShaderLibrary->Load("Assets/Shaders/DefaultLitShader.glsl");
		s_Data.ShaderLibrary->Load("Assets/Shaders/DefaultShader.glsl");
		s_Data.ShaderLibrary->Load("Assets/Shaders/LineShader.glsl");

		s_Data.Resources.Init();
		
		//s_Data.ShaderLibrary->Load("Assets/Shaders/Particle/ParticleShaderCPU.glsl");
		WaitAndRenderAll();
	}

	void Renderer::Shutdown()
	{	
		s_Data.FullscreenQuadVertexBuffer.Reset();
		s_Data.FullscreenQuadIndexBuffer.Reset();
		s_Data.ShaderLibrary.Reset();
		s_Data.ShaderDependencies.Clear();
		s_Data.Resources.Shutdown();
		s_RendererAPI->Shutdown();
		s_Data.QueueData.Shutdown();

		delete s_RendererAPI;
		s_RendererAPI = nullptr;

		s_Data.APIContext.Reset(); // Free context to make sure it is destroyed sooner than Logger;
	}

	void Renderer::Clear()
	{
		Renderer::Submit([=]() {
			s_RendererAPI->Clear();
		});
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		Renderer::Submit([=]() {
			s_RendererAPI->SetClearColor(color);
		});
	}

	void Renderer::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		Renderer::Submit([=]() {
			s_RendererAPI->SetViewport(x, y, width, height);
		});
	}

	void Renderer::SetLineThickness(float thickness)
	{
		Renderer::Submit([=]() {
			s_RendererAPI->SetLineThickness(thickness);
		});
	}

	void Renderer::SetPointSize(float size)
	{
		Renderer::Submit([=]() {
			s_RendererAPI->SetPointSize(size);
		});
	}

	void Renderer::SetDepthTest(bool val)
	{
		Renderer::Submit([=]() {
			s_RendererAPI->SetDepth(val);
		});
	}

	void Renderer::DrawArrays(PrimitiveType type, uint32_t count)
	{
		s_Data.Stats.DrawArraysCount++;
		Renderer::Submit([=]() {
			s_RendererAPI->DrawArrays(type, count);
		});
	}

	void Renderer::DrawIndexed(PrimitiveType type, uint32_t indexCount)
	{
		s_Data.Stats.DrawIndexedCount++;
		Renderer::Submit([=]() {
			s_RendererAPI->DrawIndexed(type, indexCount);
		});
	}

	void Renderer::DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset)
	{
		s_Data.Stats.DrawInstancedCount++;

		Renderer::Submit([=]() {
			s_RendererAPI->DrawInstanced(type, indexCount, instanceCount, offset);
		});
	}

	void Renderer::DrawElementsIndirect(void* indirect)
	{
		s_Data.Stats.DrawIndirectCount++;
		Renderer::Submit([=]() {
			s_RendererAPI->DrawInstancedIndirect(indirect);
		});
	}

	void Renderer::SubmitFullscreenQuad()
	{
		s_Data.Stats.DrawFullscreenCount++;
		
	}

	void Renderer::BeginFrame()
	{
		s_RendererAPI->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		s_RendererAPI->EndFrame();
	}

	void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, const Ref<RenderPass>& renderPass,
		bool clear)
	{
		XYZ_ASSERT(renderPass.Raw(), "Render pass can not be null");
		s_RendererAPI->BeginRenderPass(renderCommandBuffer, renderPass, clear);
	}

	void Renderer::EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer)
	{
		s_RendererAPI->EndRenderPass(renderCommandBuffer);
	}

	void Renderer::RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline,
		Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform, uint32_t indexCount)
	{
		s_RendererAPI->RenderGeometry(renderCommandBuffer, pipeline, material, vertexBuffer, indexBuffer, transform, indexCount);
	}

	void Renderer::SubmitFullscreenQuad(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
	{
		s_RendererAPI->RenderGeometry(renderCommandBuffer, pipeline, material, s_Data.FullscreenQuadVertexBuffer, s_Data.FullscreenQuadIndexBuffer);
	}

	void Renderer::BindPipeline(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material)
	{
		s_RendererAPI->BindPipeline(renderCommandBuffer, pipeline, uniformBufferSet, material);
	}

	void Renderer::RegisterShaderDependency(const Ref<Shader>& shader, const Ref<Pipeline>& pipeline)
	{
		s_Data.ShaderDependencies.Register(shader->GetHash(), pipeline);
	}
	void Renderer::RegisterShaderDependency(const Ref<Shader>& shader, const Ref<Material>& material)
	{
		s_Data.ShaderDependencies.Register(shader->GetHash(), material);
	}
	void Renderer::RemoveShaderDependency(size_t hash)
	{
		s_Data.ShaderDependencies.RemoveDependency(hash);
	}
	void Renderer::OnShaderReload(size_t hash)
	{
		s_Data.ShaderDependencies.OnReload(hash);
	}
	void Renderer::BlockRenderThread()
	{
		s_Data.QueueData.BlockRenderThread();
	}

	void Renderer::WaitAndRenderAll()
	{
		BlockRenderThread();
		Render();
		BlockRenderThread();
		Render();
		BlockRenderThread();
	}

	void Renderer::WaitAndRender()
	{
		Render();
		BlockRenderThread();
	}

	ThreadPool& Renderer::GetPool()
	{
		return s_Data.QueueData.GetThreadPool();
	}

	RendererAPI* Renderer::GetRendererAPI()
	{
		return s_RendererAPI;
	}

	const RendererStats& Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::Render()
	{
		s_Data.Stats.Reset();
		s_Data.QueueData.ExecuteRenderQueue();
	}
	void Renderer::ExecuteResources()
	{
		uint32_t currentFrame = s_Data.APIContext->GetCurrentFrame();
		s_Data.QueueData.ExecuteResourceQueue(currentFrame);
	}
	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}
	Ref<APIContext> Renderer::GetAPIContext()
	{
		return s_Data.APIContext;
	}
	const RendererResources& Renderer::GetDefaultResources()
	{
		return s_Data.Resources;
	}

	const RenderAPICapabilities& Renderer::GetCapabilities()
	{
		return s_RendererAPI->GetCapabilities();
	}

	const RendererConfiguration& Renderer::GetConfiguration()
	{
		return s_Data.Configuration;
	}

	ScopedLock<RenderCommandQueue> Renderer::getResourceQueue()
	{
		return s_Data.QueueData.GetResourceQueue(s_Data.APIContext->GetCurrentFrame());
	}

	RenderCommandQueue& Renderer::getRenderCommandQueue()
	{
		return s_Data.QueueData.GetRenderCommandQueue();
	}
	RendererStats& Renderer::getStats()
	{
		return s_Data.Stats;
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
	void RendererResources::Init()
	{
		auto shaderLibrary = Renderer::GetShaderLibrary();

		uint32_t whiteTextureData = 0xffffffff;
		WhiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &whiteTextureData);
		DefaultQuadMaterial = Material::Create(shaderLibrary->Get("DefaultShader"));
		DefaultLineMaterial = Material::Create(shaderLibrary->Get("LineShader"));
		DefaultCircleMaterial = Material::Create(shaderLibrary->Get("Circle"));
	}
	void RendererResources::Shutdown()
	{
		WhiteTexture.Reset();
		DefaultQuadMaterial.Reset();
		DefaultLineMaterial.Reset();
		DefaultCircleMaterial.Reset();
	}
}
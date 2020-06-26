#include "stdafx.h"
#include "RendererBatchSystem2D.h"
#include "RenderCommandQueue.h"
#include "Renderer2D.h"
#include "RenderCommand.h"


#include "XYZ/Timer.h"

namespace XYZ {
	struct Vertex2D
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
		float	  TextureID;
	};

	struct MaterialComparator
	{
		bool operator()(const std::shared_ptr<Material>& a, const std::shared_ptr<Material>& b) const
		{
			return (int16_t)a->GetSortKey() < (int16_t)b->GetSortKey();
		}
	};

	struct Renderer2DData
	{
		Renderer2DData() { Reset(); }
		void Submit(const Renderable2D& renderable,const glm::mat4& transform);
		void Reset();


		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;

		std::shared_ptr<Material> Material;
		std::shared_ptr<VertexArray> QuadVertexArray;
		std::shared_ptr<VertexBuffer> QuadVertexBuffer;

		uint32_t IndexCount = 0;
		Vertex2D* BufferBase = nullptr;
		Vertex2D* BufferPtr = nullptr;


		glm::vec4 QuadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

	};
	static std::map<std::shared_ptr<Material>, Renderer2DData, MaterialComparator> s_OpaqueBuckets;
	static std::map<std::shared_ptr<Material>, Renderer2DData, MaterialComparator> s_TransparentBuckets;

	static void SubmitCommand(const Renderer2DData& data)
	{
		uint32_t dataSize = (uint8_t*)data.BufferPtr - (uint8_t*)data.BufferBase;
		if (dataSize != 0)
		{
			data.QuadVertexBuffer->Update(data.BufferBase, dataSize);
			data.Material->Bind();
			data.QuadVertexArray->Bind();
			RenderCommand::DrawIndexed(data.QuadVertexArray, data.IndexCount);
		}
	}
	static void SubmitMaterial(std::shared_ptr<Material> material)
	{
		material->Bind();
	}
	RendererBatchSystem2D::RendererBatchSystem2D()
	{
		m_Signature.set(ECSManager::Get().GetComponentType<Renderable2D>());
		m_Signature.set(ECSManager::Get().GetComponentType<Transform2D>());
	}


	void RendererBatchSystem2D::Add(Entity entity)
	{		
		auto renderable = ECSManager::Get().GetComponent<Renderable2D>(entity);
		auto key = renderable->Material->GetSortKey();

		if (!(key & RenderFlags::InstancedFlag))
		{
			XYZ_LOG_INFO("Entity with id ", entity, " added");
			Component component;
	
			component.Renderable = ECSManager::Get().GetComponent<Renderable2D>(entity);
			component.Transform = ECSManager::Get().GetComponent<Transform2D>(entity);
			component.ActiveComponent = ECSManager::Get().GetComponent<ActiveComponent>(entity);

			if (renderable->Material->GetSortKey() & RenderFlags::TransparentFlag)
			{
				m_TransparentComponents.push_back(component);
			}
			else
			{
				m_OpaqueComponents.push_back(component);
			}
		}
	}

	void RendererBatchSystem2D::Remove(Entity entity)
	{
		if (ECSManager::Get().Contains<Renderable2D>(entity))
		{
			auto renderable = ECSManager::Get().GetComponent<Renderable2D>(entity);
			auto key = renderable->Material->GetSortKey();
			if (key & RenderFlags::TransparentFlag)
			{
				auto it = std::find(m_TransparentComponents.begin(), m_TransparentComponents.end(), entity);
				if (it != m_TransparentComponents.end())
				{
					XYZ_LOG_INFO("Entity with id ", entity, " removed");
					*it = std::move(m_TransparentComponents.back());
					m_TransparentComponents.pop_back();
				}
			}
			else
			{
				auto it = std::find(m_OpaqueComponents.begin(), m_OpaqueComponents.end(), entity);
				if (it != m_OpaqueComponents.end())
				{
					XYZ_LOG_INFO("Entity with id ", entity, " removed");
					*it = std::move(m_OpaqueComponents.back());
					m_OpaqueComponents.pop_back();
				}
			}
		}
	}
	bool RendererBatchSystem2D::Contains(Entity entity)
	{
		if (ECSManager::Get().Contains<Renderable2D>(entity))
		{
			auto renderable = ECSManager::Get().GetComponent<Renderable2D>(entity);
			auto key = renderable->Material->GetSortKey();
			if (key & RenderFlags::TransparentFlag)
			{
				auto it = std::find(m_TransparentComponents.begin(), m_TransparentComponents.end(), entity);
				if (it != m_TransparentComponents.end())
					return true;
			}
			else
			{
				auto it = std::find(m_OpaqueComponents.begin(), m_OpaqueComponents.end(), entity);
				if (it != m_OpaqueComponents.end())
					return true;
			}
		}
		return false;
	}

	// If renderable is updated and keys do not match, reinsert it and update key
	void RendererBatchSystem2D::EntityUpdated(Entity entity)
	{
		// TODO
	}

	void RendererBatchSystem2D::SubmitToRenderer()
	{	
		for (auto it : m_OpaqueComponents)
		{
			auto material = it.Renderable->Material;
			auto& bucket = s_OpaqueBuckets[material];
			if (bucket.IndexCount < bucket.IndexCount)
			{
				bucket.Submit(*it.Renderable,it.Transform->GetTransformation());
			}
			else
			{
				bucket.Material = material;
				SubmitCommand(bucket);
				bucket.Reset();
			}
		}
		for (auto& it : s_OpaqueBuckets)
		{
			it.second.Material = it.first;
			SubmitCommand(it.second);
			it.second.Reset();
		}
		
		for (auto it : m_TransparentComponents)
		{		
			auto material = it.Renderable->Material;
			auto& bucket = s_TransparentBuckets[material];
			
			if (bucket.IndexCount < bucket.MaxIndices)
			{
				bucket.Submit(*it.Renderable,it.Transform->GetTransformation());
			}
			else
			{
				bucket.Material = material;
				SubmitCommand(bucket);
				bucket.Reset();
			}
		}
		for (auto& it : s_TransparentBuckets)
		{
			it.second.Material = it.first;
			SubmitCommand(it.second);
			it.second.Reset();
		}
	}


	void Renderer2DData::Submit(const Renderable2D& renderable,const glm::mat4& transform)
	{
		constexpr size_t quadVertexCount = 4;
		auto& texCoordPack = renderable.SubTexture->GetTexCoords();
		
		glm::vec2 texCoords[quadVertexCount] = {
			{texCoordPack.x,texCoordPack.y},
			{texCoordPack.z,texCoordPack.y},
			{texCoordPack.z,texCoordPack.w},
			{texCoordPack.x,texCoordPack.w}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			BufferPtr->Position = transform * QuadVertexPositions[i];
			BufferPtr->Color = renderable.Color;
			BufferPtr->TexCoord = texCoords[i];
			BufferPtr->TextureID = (float)renderable.TextureID;
			BufferPtr++;
		}
		IndexCount += 6;
	}

	void Renderer2DData::Reset()
	{
		if (!BufferBase)
		{
			BufferBase = new Vertex2D[MaxVertices];
			QuadVertexArray = VertexArray::Create();
			QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(Vertex2D));
			QuadVertexBuffer->SetLayout(BufferLayout{
			{0, XYZ::ShaderDataType::Float4, "a_Color" },
			{1, XYZ::ShaderDataType::Float3, "a_Position" },
			{2, XYZ::ShaderDataType::Float2, "a_TexCoord" },
			{3, XYZ::ShaderDataType::Float,  "a_TextureID" },
				});
			QuadVertexArray->AddVertexBuffer(QuadVertexBuffer);


			uint32_t* quadIndices = new uint32_t[MaxIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			std::shared_ptr<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, MaxIndices);
			QuadVertexArray->SetIndexBuffer(quadIB);
			delete[] quadIndices;
		}
		BufferPtr = BufferBase;
		IndexCount = 0;
	}

}
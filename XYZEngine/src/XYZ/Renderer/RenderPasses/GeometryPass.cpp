#include "stdafx.h"
#include "GeometryPass.h"


namespace XYZ {
	GeometryPass3D::GeometryPass3D()
	{
	}
	void GeometryPass3D::Init(const GeometryPassConfiguration& config)
	{
		m_RenderPass = config.Pass;
		m_CameraBufferSet = config.CameraBufferSet;
		m_InstanceVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_InstanceVertexBufferSize);
		m_TransformVertexBufferSet = Ref<VertexBufferSet>::Create(Renderer::GetConfiguration().FramesInFlight, sc_TransformBufferSize);
		m_BoneTransformsStorageSet = StorageBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		m_BoneTransformsStorageSet->Create(sc_MaxBoneTransforms * sizeof(GeometryRenderQueue::BoneTransforms), 2, 0);
	
		m_TransformData.resize(GetTransformBufferCount());
		m_BoneTransformsData.resize(sc_MaxBoneTransforms);
		m_InstanceData.resize(sc_InstanceVertexBufferSize);
	}
	void GeometryPass3D::Submit(const Ref<RenderCommandBuffer>& commandBuffer, GeometryRenderQueue& queue, bool clear)
	{
		XYZ_PROFILE_FUNC("GeometryPass3D::Submit");
		Renderer::BeginRenderPass(commandBuffer, m_RenderPass, clear);

		submitStaticMeshes(queue, commandBuffer);
		submitAnimatedMeshes(queue, commandBuffer);
		submitInstancedMeshes(queue, commandBuffer);
		
		Renderer::EndRenderPass(commandBuffer);
	}

	GeometryPassStatistics GeometryPass3D::PreSubmit(GeometryRenderQueue& queue)
	{
		XYZ_PROFILE_FUNC("GeometryPass3D::PreSubmit");
		size_t overrideCount = 0;
		uint32_t transformsCount = 0;
		uint32_t boneTransformCount = 0;
		uint32_t instanceOffset = 0;

		prepareStaticDrawCommands(queue, overrideCount, transformsCount);
		prepareAnimatedDrawCommands(queue, overrideCount, transformsCount, boneTransformCount);
		prepareInstancedDrawCommands(queue, instanceOffset);

		m_TransformVertexBufferSet->Update(m_TransformData.data(), transformsCount * sizeof(GeometryRenderQueue::TransformData));
		m_InstanceVertexBufferSet->Update(m_InstanceData.data(), instanceOffset);
		m_BoneTransformsStorageSet->Update(m_BoneTransformsData.data(), boneTransformCount * sizeof(GeometryRenderQueue::BoneTransforms), 0, 0, 2);

		return { static_cast<uint32_t>(overrideCount), transformsCount, instanceOffset };
	}

	void GeometryPass3D::submitStaticMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, command] : queue.MeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_CameraBufferSet,
				nullptr,
				command.MaterialAsset->GetMaterial()
			);

			Renderer::RenderMesh(
				commandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				glm::mat4(1.0f),
				m_TransformVertexBufferSet,
				command.TransformOffset,
				command.TransformInstanceCount
			);
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					commandBuffer,
					command.Pipeline,
					dcOverride.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					dcOverride.Transform
				);
			}
		}
	}
	void GeometryPass3D::submitAnimatedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		for (auto& [key, command] : queue.AnimatedMeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_CameraBufferSet,
				m_BoneTransformsStorageSet,
				command.MaterialAsset->GetMaterial()
			);
			Renderer::RenderMesh(
				commandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				{ glm::mat4(1.0f), command.BoneTransformsIndex },
				m_TransformVertexBufferSet,
				command.TransformOffset,
				command.TransformInstanceCount
			);
			for (auto& dcOverride : command.OverrideCommands)
			{
				Renderer::RenderMesh(
					commandBuffer,
					command.Pipeline,
					dcOverride.OverrideMaterial,
					command.Mesh->GetVertexBuffer(),
					command.Mesh->GetIndexBuffer(),
					{ dcOverride.Transform, dcOverride.BoneTransformsIndex },
					m_TransformVertexBufferSet,
					command.TransformOffset,
					command.TransformInstanceCount
				);
			}
		}
	}
	void GeometryPass3D::submitInstancedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		uint32_t index = Renderer::GetCurrentFrame();
		for (auto& [key, command] : queue.InstanceMeshDrawCommands)
		{
			Renderer::BindPipeline(
				commandBuffer,
				command.Pipeline,
				m_CameraBufferSet,
				nullptr,
				command.MaterialAsset->GetMaterial()
			);

			Renderer::RenderMesh(
				commandBuffer,
				command.Pipeline,
				command.OverrideMaterial,
				command.Mesh->GetVertexBuffer(),
				command.Mesh->GetIndexBuffer(),
				command.Transform,
				m_InstanceVertexBufferSet,
				command.InstanceOffset,
				command.InstanceCount
			);
		}
	}
	void GeometryPass3D::prepareStaticDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount)
	{	
		for (auto& [key, dc] : queue.MeshDrawCommands)
		{
			dc.Pipeline = prepareGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.TransformOffset = transformsCount * sizeof(GeometryRenderQueue::TransformData);
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
		}
	}
	void GeometryPass3D::prepareAnimatedDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount, uint32_t& boneTransformCount)
	{
		for (auto& [key, dc] : queue.AnimatedMeshDrawCommands)
		{
			dc.Pipeline = prepareGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.TransformOffset = transformsCount * sizeof(TransformData);
			dc.BoneTransformsIndex = boneTransformCount;
			overrideCount += dc.OverrideCommands.size();
			for (const auto& transform : dc.TransformData)
			{
				m_TransformData[transformsCount] = transform;
				transformsCount++;
			}
			for (const auto& bones : dc.BoneData)
			{
				const size_t offset = boneTransformCount * bones.size();
				memcpy(&m_BoneTransformsData[offset], bones.data(), sizeof(GeometryRenderQueue::BoneTransforms));
				boneTransformCount++;
			}
			for (auto& overrideDc : dc.OverrideCommands)
			{
				const auto& bones = overrideDc.BoneTransforms;
				const size_t offset = boneTransformCount * bones.size();
				memcpy(&m_BoneTransformsData[offset], bones.data(), sizeof(GeometryRenderQueue::BoneTransforms));
				overrideDc.BoneTransformsIndex = boneTransformCount;
				boneTransformCount++;
			}
		}
	}
	void GeometryPass3D::prepareInstancedDrawCommands(GeometryRenderQueue& queue, uint32_t& instanceOffset)
	{
		for (auto& [key, dc] : queue.InstanceMeshDrawCommands)
		{
			dc.Pipeline = prepareGeometryPipeline(dc.MaterialAsset->GetMaterial(), dc.MaterialAsset->IsOpaque());
			dc.InstanceOffset = instanceOffset;
			memcpy(&m_InstanceData.data()[instanceOffset], dc.InstanceData.data(), dc.InstanceData.size());
			instanceOffset += dc.InstanceData.size();
		}
	}
	Ref<Pipeline> GeometryPass3D::prepareGeometryPipeline(const Ref<Material>& material, bool opaque)
	{
		Ref<Shader> shader = material->GetShader();
		auto it = m_GeometryPipelines.find(shader->GetHash());
		if (it != m_GeometryPipelines.end())
			return it->second;


		PipelineSpecification spec;
		spec.Layouts = shader->GetLayouts();
		spec.RenderPass = m_RenderPass;
		spec.Shader = shader;
		spec.Topology = PrimitiveTopology::Triangles;
		spec.DepthTest = true;
		spec.DepthWrite = true;

		auto& pipeline = m_GeometryPipelines[shader->GetHash()];
		pipeline = Pipeline::Create(spec);
		return pipeline;
	}
	
	GeometryPass2D::GeometryPass2D()	
	{
		
	}

	void GeometryPass2D::Init(const GeometryPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		m_RenderPass = config.Pass;
		m_CameraBufferSet = config.CameraBufferSet;
		m_Renderer2D = Ref<Renderer2D>::Create(Renderer2DConfiguration{ commandBuffer, m_RenderPass, m_CameraBufferSet });
		m_WhiteTexture = Renderer::GetDefaultResources().WhiteTexture;
	}

	void GeometryPass2D::Submit(const Ref<RenderCommandBuffer>& commandBuffer, GeometryRenderQueue& queue, const glm::mat4& viewMatrix, bool clear)
	{
		XYZ_PROFILE_FUNC("GeometryPass2D::Submit");

		m_Renderer2D->SetTargetRenderPass(m_RenderPass);

		m_Renderer2D->BeginScene(viewMatrix, clear);

		for (auto& [key, command] : queue.SpriteDrawCommands)
		{
			m_Renderer2D->SetQuadMaterial(command.Material);
			Ref<Material> material = command.Material->GetMaterial();
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

			for (const auto& data : command.SpriteData)
				m_Renderer2D->SubmitQuad(data.Transform, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->Flush();
		}

		for (auto& [key, command] : queue.BillboardDrawCommands)
		{
			m_Renderer2D->SetQuadMaterial(command.Material);
			Ref<Material> material = command.Material->GetMaterial();
			for (uint32_t i = 0; i < command.TextureCount; ++i)
				material->SetImageArray("u_Texture", command.Textures[i]->GetImage(), i);
			for (uint32_t i = command.TextureCount; i < Renderer2D::GetMaxTextures(); ++i)
				material->SetImageArray("u_Texture", m_WhiteTexture->GetImage(), i);

			for (const auto& data : command.BillboardData)
				m_Renderer2D->SubmitQuadBillboard(data.Position, data.Size, data.TexCoords, data.TextureIndex, data.Color);

			m_Renderer2D->Flush();
		}

		m_Renderer2D->EndScene();
	}
}
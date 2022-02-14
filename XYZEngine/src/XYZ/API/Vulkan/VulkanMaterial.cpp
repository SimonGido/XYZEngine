#include "stdafx.h"
#include "VulkanMaterial.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {

	namespace Utils {
		static uint32_t s_NextID = 0;
		static std::queue<uint32_t> s_FreeIDs;
		
		static uint32_t GenerateID()
		{
			if (!s_FreeIDs.empty())
			{
				uint32_t id = s_FreeIDs.back();
				s_FreeIDs.pop();
				return id;
			}
			return s_NextID++;
		}
	}

	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
		:
		m_Shader(shader),
		m_WriteDescriptors(Renderer::GetConfiguration().FramesInFlight),
		m_DescriptorsDirty(true),
		m_ID(Utils::GenerateID())
	{
		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->Invalidate();
		});
		Renderer::RegisterShaderDependency(shader, this);
	}

	VulkanMaterial::~VulkanMaterial()
	{	
		Utils::s_FreeIDs.push(m_ID);
	}
	void VulkanMaterial::Invalidate()
	{
		m_DescriptorsDirty = true;
		invalidateInstances();

		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		m_Descriptors.resize(Renderer::GetConfiguration().FramesInFlight);
		
		m_ImageDescriptors.clear();
		m_ImageArraysDescriptors.clear();

		m_ImageDescriptors.resize(shaderDescriptorSets.size());
		m_ImageArraysDescriptors.resize(shaderDescriptorSets.size());

		for (auto& descriptor : m_Descriptors) // Per frame
		{
			descriptor.DescriptorSets.resize(shaderDescriptorSets.size());
			for (uint32_t set = 0; set < descriptor.DescriptorSets.size(); ++set)
				descriptor.DescriptorSets[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
		}
		for (uint32_t frame = 0; frame < Renderer::GetConfiguration().FramesInFlight; ++frame)
		{
			m_Descriptors[frame].Version = VulkanRendererAPI::GetDescriptorAllocatorVersion(frame);
		}
	}
	void VulkanMaterial::SetFlag(RenderFlags renderFlag, bool val)
	{
		if (val)
		{
			m_Flags.Set(renderFlag);
		}
		else
		{
			m_Flags.Unset(renderFlag);
		}
	}


	void VulkanMaterial::SetImageArray(const std::string& name, Ref<Image2D> image, uint32_t arrayIndex)
	{
		setDescriptor(name, arrayIndex, image);
	}

	void VulkanMaterial::SetImage(const std::string& name, Ref<Image2D> image, int32_t mip)
	{
		setDescriptor(name, image, mip);
	}

	void VulkanMaterial::RT_UpdateForRendering(const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors, const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors,
		bool forceDescriptorAllocation)
	{	
		const uint32_t frame = Renderer::GetCurrentFrame();
		bool allocated = tryAllocateDescriptorSets(forceDescriptorAllocation);

		if (m_DescriptorsDirty || allocated)
		{
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			m_ArrayImageInfos.clear();
			m_ArrayImageInfos.resize(framesInFlight);
			m_DescriptorsDirty = false;

			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				RT_updateForRenderingFrame(frame, m_ArrayImageInfos[frame], uniformBufferDescriptors, storageBufferDescriptors);
			}
		}
	
		auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		vkUpdateDescriptorSets(vulkanDevice, (uint32_t)m_WriteDescriptors[frame].size(), m_WriteDescriptors[frame].data(), 0, nullptr);		
	}


	void VulkanMaterial::RT_updateForRenderingFrame(uint32_t frame,
		vector2D<VkDescriptorImageInfo>& arrayImageInfos,
		const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors,
		const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors
	)
	{
		Ref<VulkanShader> vulkanShader = m_Shader;
		const uint32_t numSets = vulkanShader->GetDescriptorSets().size();

		m_WriteDescriptors[frame].clear();
		arrayImageInfos.resize(numSets);
		for (uint32_t set = 0; set < numSets; ++set)
		{
			for (auto& pending : m_ImageDescriptors[set])
			{
				if (pending.Image.Raw())
				{
					pending.WriteDescriptor.pImageInfo = pending.Mip == -1 ? &pending.Image->GetDescriptor()
						: &pending.Image->RT_GetMipImageDescriptor(pending.Mip);
					pending.WriteDescriptor.dstSet = m_Descriptors[frame].DescriptorSets[set];
					m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
				}
			}
			for (auto& pending : m_ImageArraysDescriptors[set])
			{
				if (pending.Images.empty())
					continue;

				for (auto& image : pending.Images)
				{
					arrayImageInfos[set].push_back(image->GetDescriptor());
				}
				pending.WriteDescriptor.pImageInfo = arrayImageInfos[set].data();
				pending.WriteDescriptor.descriptorCount = arrayImageInfos[set].size();
				pending.WriteDescriptor.dstSet = m_Descriptors[frame].DescriptorSets[set];
				m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
			}

			if (!uniformBufferDescriptors.empty())
			{
				for (auto& desc : uniformBufferDescriptors[frame][set])
				{
					m_WriteDescriptors[frame].push_back(desc);
					auto& last = m_WriteDescriptors[frame].back();
					last.dstSet = m_Descriptors[frame].DescriptorSets[set];
				}
			}
			if (!storageBufferDescriptors.empty())
			{
				for (auto& desc : storageBufferDescriptors[frame][set])
				{
					m_WriteDescriptors[frame].push_back(desc);
					auto& last = m_WriteDescriptors[frame].back();
					last.dstSet = m_Descriptors[frame].DescriptorSets[set];
				}
			}
		}
	}

	bool VulkanMaterial::tryAllocateDescriptorSets(bool force)
	{
		const uint32_t frame = Renderer::GetCurrentFrame();
		VulkanDescriptorAllocator::Version newVersion = VulkanRendererAPI::GetDescriptorAllocatorVersion(frame);
		if (newVersion != m_Descriptors[frame].Version || force)
		{
			m_Descriptors[frame].Version = newVersion;
			Ref<VulkanShader> vulkanShader = m_Shader;
			const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		
			auto& descriptorSet = m_Descriptors[frame].DescriptorSets;
			for (uint32_t set = 0; set < descriptorSet.size(); ++set)
			{
				descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
			}
			return true;
		}
	}


	void VulkanMaterial::setDescriptor(const std::string& name, Ref<Image2D> image, int32_t mip)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();

		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;
		Renderer::Submit([instance, vulkanImage, textureDescriptor, textureSet, binding, mip]() mutable {

			if (binding >= instance->m_Images.size())
				instance->m_Images.resize(static_cast<size_t>(binding) + 1);
			instance->m_Images[binding] = vulkanImage;

			if (instance->m_ImageDescriptors[textureSet].size() <= binding)
				instance->m_ImageDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);

			auto& desc = instance->m_ImageDescriptors[textureSet][binding];

			desc.Image = vulkanImage;
			desc.WriteDescriptor = textureDescriptor;
			desc.Mip = mip;
			instance->m_DescriptorsDirty = true;
		});
	}
	void VulkanMaterial::setDescriptor(const std::string& name, uint32_t index, Ref<Image2D> image)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();
		
		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;
		Renderer::Submit([instance, vulkanImage, textureDescriptor, textureSet, binding, index]() mutable {

			if (binding >= instance->m_ImageArrays.size())
				instance->m_ImageArrays.resize(static_cast<size_t>(binding) + 1);
			if (index >= instance->m_ImageArrays[binding].size())
				instance->m_ImageArrays[binding].resize(static_cast<size_t>(index) + 1);

			instance->m_ImageArrays[binding][index] = vulkanImage;

			if (instance->m_ImageArraysDescriptors[textureSet].size() <= binding)
				instance->m_ImageArraysDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);

			if (instance->m_ImageArraysDescriptors[textureSet][binding].Images.size() <= index)
				instance->m_ImageArraysDescriptors[textureSet][binding].Images.resize(static_cast<size_t>(index) + 1);


			auto& desc = instance->m_ImageArraysDescriptors[textureSet][binding];
			desc.Images[index] = vulkanImage;
			desc.WriteDescriptor = textureDescriptor;

			instance->m_DescriptorsDirty = true;
		});
	}


	const ShaderResourceDeclaration* VulkanMaterial::findResourceDeclaration(const std::string& name)
	{
		auto& resources = m_Shader->GetResources();
		for (const auto& [n, resource] : resources)
		{
			if (resource.GetName() == name)
				return &resource;
		}
		return nullptr;
	}
}
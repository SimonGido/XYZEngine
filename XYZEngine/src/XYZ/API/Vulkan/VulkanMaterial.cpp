#include "stdafx.h"
#include "VulkanMaterial.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {

	
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
		:
		m_Shader(shader),
		m_WriteDescriptors(Renderer::GetConfiguration().FramesInFlight),
		m_DescriptorsDirty(true),
		m_AllocateDescriptors(true)
	{
		Invalidate();
		Renderer::RegisterShaderDependency(shader, this);
	}

	VulkanMaterial::~VulkanMaterial()
	{	
	}
	void VulkanMaterial::Invalidate()
	{
		m_DescriptorsDirty = true;
		m_AllocateDescriptors = true;

		invalidateInstances();		

		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		m_Descriptors.resize(Renderer::GetConfiguration().FramesInFlight);
		
		for (auto& descriptor : m_Descriptors) // Per frame
		{
			descriptor.DescriptorSets.resize(shaderDescriptorSets.size());
		};

		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance]() mutable {
			
			instance->m_ImageDescriptors.clear();
			instance->m_ImageArrayDescriptors.clear();
		});

		if (m_OnInvalidate)
			m_OnInvalidate();
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

	void VulkanMaterial::RT_UpdateForRendering(
		const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors, 
		const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors,
		bool forceDescriptorAllocation
	)
	{	
		bool descriptorsAllocated = false;

		if (m_AllocateDescriptors) // Material shader updated, allocate descriptors for every frame
		{
			allocateDescriptorSetsAll();
			descriptorsAllocated = true;
			m_AllocateDescriptors = false;
		}
		else
		{
			// Allocate descriptor set for current frame if it's version has changed
			descriptorsAllocated = allocateDescriptorSetsFrame(forceDescriptorAllocation);
		}

		if (m_DescriptorsDirty || descriptorsAllocated)
		{
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			m_DescriptorsDirty = false;

			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				RT_updateForRenderingFrame(frame);
			}
		}

		const uint32_t currentFrame = Renderer::GetCurrentFrame();
		uint32_t resourceDescriptorCount = m_WriteDescriptors[currentFrame].ResourceWriteDescriptorCount;
		auto& writeDescriptors = m_WriteDescriptors[currentFrame].WriteDescriptors;
		writeDescriptors.resize(resourceDescriptorCount);

		
		if (!uniformBufferDescriptors.empty())
		{
			uint32_t set = 0;
			for (auto& frameDesc : uniformBufferDescriptors[currentFrame])
			{
				for (auto& desc : frameDesc)
				{
					writeDescriptors.push_back(desc);
					auto& last = writeDescriptors.back();
					last.dstSet = m_Descriptors[currentFrame].DescriptorSets[set];
				}
				set++;
			}
		}
		if (!storageBufferDescriptors.empty())
		{
			uint32_t set = 0;
			for (auto& frameDesc : storageBufferDescriptors[currentFrame])
			{
				for (auto& desc : frameDesc)
				{
					writeDescriptors.push_back(desc);
					auto& last = writeDescriptors.back();
					last.dstSet = m_Descriptors[currentFrame].DescriptorSets[set];
				}
				set++;
			}
		}

		glm::ivec2 scr;
		if (!m_MaterialInstances.empty())
		{
			if ((*m_MaterialInstances.begin())->HasUniform("u_ScreenData.u_ScreenSize"))
			{
				scr = (*m_MaterialInstances.begin())->Get<glm::ivec2>("u_ScreenData.u_ScreenSize");
			}
		}
		auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		vkUpdateDescriptorSets(vulkanDevice, (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}


	void VulkanMaterial::RT_updateForRenderingFrame(uint32_t frame)
	{
		Ref<VulkanShader> vulkanShader = m_Shader;
		const uint32_t numSets = vulkanShader->GetDescriptorSets().size();

		m_WriteDescriptors[frame].WriteDescriptors.clear();
		m_WriteDescriptors[frame].ResourceWriteDescriptorCount = 0;

		for (auto &[set, descriptors] : m_ImageDescriptors)
		{
			for (auto& [binding, pending] : descriptors)
			{
				pending.WriteDescriptor.pImageInfo = pending.Mip == -1 ? &pending.Image->GetDescriptor()
					: &pending.Image->RT_GetMipImageDescriptor(pending.Mip);

				pending.WriteDescriptor.dstSet = m_Descriptors[frame].DescriptorSets[set];
				m_WriteDescriptors[frame].WriteDescriptors.push_back(pending.WriteDescriptor);
			}
		}
		for (auto& [set, descriptors] : m_ImageArrayDescriptors)
		{
			for (auto& [binding, pending] : descriptors)
			{
				if (pending.Images.empty())
					continue;

				pending.ImagesInfo.resize(pending.Images.size());
				for (size_t i = 0; i < pending.Images.size(); ++i)
				{
					pending.ImagesInfo[i] = pending.Images[i]->GetDescriptor();
				}

				pending.WriteDescriptor.pImageInfo = pending.ImagesInfo.data();
				pending.WriteDescriptor.descriptorCount = static_cast<uint32_t>(pending.ImagesInfo.size());
				pending.WriteDescriptor.dstSet = m_Descriptors[frame].DescriptorSets[set];
				m_WriteDescriptors[frame].WriteDescriptors.push_back(pending.WriteDescriptor);
			}
		}
		m_WriteDescriptors[frame].ResourceWriteDescriptorCount = static_cast<uint32_t>(m_WriteDescriptors[frame].WriteDescriptors.size());
	}
	bool VulkanMaterial::allocateDescriptorSetsFrame(bool force)
	{
		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		const uint32_t frame = Renderer::GetCurrentFrame();
		const auto newVersion = VulkanRendererAPI::GetDescriptorAllocatorVersion(frame);
		
		if (m_Descriptors[frame].Version != newVersion || force)
		{
			m_Descriptors[frame].Version = newVersion;
			auto& descriptorSet = m_Descriptors[frame].DescriptorSets;
			for (uint32_t set = 0; set < descriptorSet.size(); ++set)
			{
				descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
			}
			return true;
		}
	}
	void VulkanMaterial::allocateDescriptorSetsAll()
	{
		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		
		uint32_t frame = 0;
		for (auto& descriptor : m_Descriptors) // Per frame
		{
			descriptor.Version = VulkanRendererAPI::GetDescriptorAllocatorVersion(frame);
			auto& descriptorSet = descriptor.DescriptorSets;
			for (uint32_t set = 0; set < descriptorSet.size(); ++set)
			{
				descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
			}
			frame++;
		}
	}


	void VulkanMaterial::setDescriptor(const std::string& name, Ref<Image2D> image, int32_t mip)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();

		auto [wds, dscSet] = m_Shader->GetDescriptorSet(name);

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;

		Renderer::Submit([instance, vulkanImage, descriptor = *wds, set = dscSet, binding, mip]() mutable {

			auto& desc = instance->m_ImageDescriptors[set][binding];
			desc.Image = vulkanImage;
			desc.WriteDescriptor = descriptor;
			desc.Mip = mip;
			instance->m_DescriptorsDirty = true;
		});
	}
	void VulkanMaterial::setDescriptor(const std::string& name, uint32_t index, Ref<Image2D> image)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();
		
		auto [wds, dscSet] = m_Shader->GetDescriptorSet(name);
		

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;

		Renderer::Submit([instance, vulkanImage, descriptor = *wds, set = dscSet, binding, index]() mutable {

			auto& desc = instance->m_ImageArrayDescriptors[set][binding];
			if (index >= desc.Images.size())
				desc.Images.resize(static_cast<size_t>(index) + 1);
	
			desc.Images[index] = vulkanImage;
			desc.WriteDescriptor = descriptor;
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
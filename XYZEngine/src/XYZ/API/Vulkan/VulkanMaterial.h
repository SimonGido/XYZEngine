#pragma once
#include "XYZ/Renderer/Material.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include "VulkanRendererAPI.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"

namespace XYZ {
	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const Ref<Shader>& shader);

		virtual ~VulkanMaterial() override;
		virtual void Invalidate() override;

		virtual void SetFlag(RenderFlags renderFlag, bool val = true) override;

		virtual void SetImageArray(const std::string& name, Ref<Image2D> image, uint32_t arrayIndex) override;
		virtual void SetImage(const std::string& name, Ref<Image2D> image, int32_t mip = -1) override;


		virtual uint64_t	   GetFlags() const override { return m_Flags.ToUlong(); }
		virtual uint32_t	   GetID()    const  override { return m_ID; };
		virtual Ref<Shader>	   GetShader() const override { return m_Shader; }


		void RT_UpdateForRendering(
			const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors, 
			const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors,
			bool forceDescriptroAllocation = false
		);
		const std::vector<VkWriteDescriptorSet>& GetWriteDescriptors(uint32_t frame) const { return m_WriteDescriptors[frame]; }
		const std::vector<VkDescriptorSet>&      GetDescriptors(uint32_t frame) const { return m_Descriptors[frame].DescriptorSets; }

	private:
		void RT_updateForRenderingFrame(uint32_t frame,
			vector2D<VkDescriptorImageInfo>& arrayImageInfos,
			const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors,
			const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors
		);

		bool tryAllocateDescriptorSets(bool force = false);

		void setDescriptor(const std::string& name, Ref<Image2D> image, int32_t mip);
		void setDescriptor(const std::string& name, uint32_t index, Ref<Image2D> image);

		const ShaderResourceDeclaration* findResourceDeclaration(const std::string& name);

	private:
		Ref<VulkanShader>					   m_Shader;

		std::vector<Ref<Image2D>>			   m_Images;
		std::vector<std::vector<Ref<Image2D>>> m_ImageArrays;
		
		vector3D<VkDescriptorImageInfo>		   m_ArrayImageInfos;
			
		struct Descriptor
		{
			// per set
			std::vector<VkDescriptorSet>	   DescriptorSets;
			VulkanDescriptorAllocator::Version Version;
		};
		// Per frame
		std::vector<Descriptor>	m_Descriptors;
		

		struct PendingDescriptor
		{
			VkWriteDescriptorSet WriteDescriptor;
			Ref<VulkanImage2D> Image;
			int32_t Mip = -1;
		};
	
		struct PendingDescriptorArray
		{
			VkWriteDescriptorSet WriteDescriptor;
			std::vector<Ref<VulkanImage2D>> Images;
		};

		// Per set -> per binding
		vector2D<PendingDescriptor> m_ImageDescriptors;

		// Per set -> per binding
		vector2D<PendingDescriptorArray> m_ImageArraysDescriptors;

		// Per frame
		vector2D<VkWriteDescriptorSet> m_WriteDescriptors;

		Flags<RenderFlags>			   m_Flags;
		bool						   m_DescriptorsDirty;

		uint32_t					   m_ID;		
	};
}
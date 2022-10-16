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
		virtual Ref<Shader>	   GetShader() const override { return m_Shader; }


		void RT_UpdateForRendering(
			const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors,
			const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors,
			bool forceDescriptorAllocation = false
		);
		const std::vector<VkWriteDescriptorSet>& GetWriteDescriptors(uint32_t frame) const { return m_WriteDescriptors[frame].WriteDescriptors; }
		const std::vector<VkDescriptorSet>& GetDescriptors(uint32_t frame) const { return m_Descriptors[frame].DescriptorSets; }

	private:
		void RT_updateForRenderingFrame(uint32_t frame);
		
		bool allocateDescriptorSetsFrame(bool force);
		void allocateDescriptorSetsAll();

		void setDescriptor(const std::string& name, Ref<Image2D> image, int32_t mip);
		void setDescriptor(const std::string& name, uint32_t index, Ref<Image2D> image);

		const ShaderResourceDeclaration* findResourceDeclaration(const std::string& name);

	private:
		Ref<VulkanShader> m_Shader;
			
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
			std::vector< VkDescriptorImageInfo> ImagesInfo;
		};

		// Per set -> per binding
		unordered_map2D<uint32_t, uint32_t, PendingDescriptor> m_ImageDescriptors;
		// Per set -> per binding
		unordered_map2D<uint32_t, uint32_t, PendingDescriptorArray> m_ImageArrayDescriptors;

		struct WriteDescriptorSet
		{
			std::vector<VkWriteDescriptorSet> WriteDescriptors;
			uint32_t						  ResourceWriteDescriptorCount;
		};

		// Per frame
		std::vector<WriteDescriptorSet> m_WriteDescriptors;

		Flags<RenderFlags>			   m_Flags;
		bool						   m_DescriptorsDirty;
		bool						   m_AllocateDescriptors;
	
	};
}
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

		virtual void Set(const std::string& name, float value) override;
		virtual void Set(const std::string& name, int value) override;

		virtual void Set(const std::string& name, const glm::vec2& value) override;
		virtual void Set(const std::string& name, const glm::vec3& value) override;
		virtual void Set(const std::string& name, const glm::vec4& value) override;
		virtual void Set(const std::string& name, const glm::mat4& value) override;

		virtual void Set(const std::string& name, const glm::ivec2& value) override;
		virtual void Set(const std::string& name, const glm::ivec3& value) override;
		virtual void Set(const std::string& name, const glm::ivec4& value) override;

		virtual void Set(const std::string& name, const Ref<Texture2D>& texture) override;
		virtual void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex) override;
		virtual void Set(const std::string& name, const Ref<Image2D>& image) override;

		virtual float&     GetFloat(const std::string& name) override;
		virtual int32_t&   GetInt(const std::string& name) override;
		virtual uint32_t&  GetUInt(const std::string& name) override;
		virtual bool&	   GetBool(const std::string& name) override;

		virtual glm::ivec2& GetIVector2(const std::string& name) override;
		virtual glm::ivec3& GetIVector3(const std::string& name) override;
		virtual glm::ivec4& GetIVector4(const std::string& name) override;

		virtual glm::vec2& GetVector2(const std::string& name) override;
		virtual glm::vec3& GetVector3(const std::string& name) override;
		virtual glm::vec4& GetVector4(const std::string& name) override;
		virtual glm::mat3& GetMatrix3(const std::string& name) override;
		virtual glm::mat4& GetMatrix4(const std::string& name) override;

		virtual Ref<Texture2D> GetTexture2D(const std::string& name) override;
	
		virtual uint64_t	   GetFlags() const override { return m_Flags.ToUlong(); }
		virtual uint32_t	   GetID()    const  override { return m_ID; };
		virtual Ref<Shader>	   GetShader() const override { return m_Shader; }


		void RT_UpdateForRendering(const vector3D<VkWriteDescriptorSet>& descriptors);
		const std::vector<VkWriteDescriptorSet>& GetWriteDescriptors(uint32_t frame) const { return m_WriteDescriptors[frame]; }
		const std::vector<VkDescriptorSet>&      GetDescriptors(uint32_t frame) const { return m_DescriptorSets[frame]; }
		const ByteBuffer						 GetFSUniformsBuffer() const;
		const ByteBuffer						 GetVSUniformsBuffer() const;
	private:
		void tryAllocateDescriptorSets();

		void allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers, ByteBuffer& buffer) const;
		void setDescriptor(const std::string& name, const Ref<Texture2D>& texture);
		void setDescriptor(const std::string& name, const Ref<Image2D>& image);
		void setDescriptor(const std::string& name, const Ref<Texture2D>& texture, uint32_t index);

		template <typename T>
		void set(const std::string& name, const T& value);

		template<typename T>
		T& get(const std::string& name);


		template<typename T>
		Ref<T> getResource(const std::string& name);
		

		std::pair<const ShaderUniform*, ByteBuffer*> findUniformDeclaration(const std::string& name);
		const ShaderResourceDeclaration*			 findResourceDeclaration(const std::string& name);

	private:
		Ref<VulkanShader>						 m_Shader;
		ByteBuffer								 m_UniformsBuffer;

		std::vector<Ref<Image2D>>				 m_Images;
		std::vector<Ref<Texture2D>>			     m_Textures;
		std::vector<std::vector<Ref<Texture2D>>> m_TextureArrays;
		// Per frame -> per set
		vector2D<VkDescriptorSet>				 m_DescriptorSets;
		VulkanDescriptorAllocator::Version		 m_DescriptorsVersion;

		struct PendingDescriptor
		{
			VkWriteDescriptorSet WriteDescriptor;
			Ref<VulkanImage2D> Image;
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

	template<typename T>
	inline void VulkanMaterial::set(const std::string& name, const T& value)
	{
		auto [decl, buffer] = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		if (!decl)
			return;
		buffer->Write((uint8_t*)&value, decl->GetSize(), decl->GetOffset());
	}
	template<typename T>
	inline T& VulkanMaterial::get(const std::string& name)
	{
		auto [decl, buffer] = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		return buffer->Read<T>(decl->GetOffset());
	}
	template<typename T>
	inline Ref<T> VulkanMaterial::getResource(const std::string& name)
	{
		auto decl = findResourceDeclaration(name);
		XYZ_ASSERT(decl, "Could not find uniform with name 'x'");
		uint32_t slot = decl->GetRegister();
		XYZ_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
		return Ref<T>(m_Textures[slot]);
	}
}
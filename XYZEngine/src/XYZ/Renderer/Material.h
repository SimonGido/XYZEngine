#pragma once
#include "Shader.h"
#include "Texture.h"
#include "RenderFlags.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <queue>
#include <bitset>
#include <unordered_set>

namespace XYZ {

	class Material : public Asset
	{
		friend class MaterialInstance;
	public:
		Material(const Ref<ShaderAsset>& shaderAsset);
		~Material();


		template<typename T>
		void Set(const std::string& name, const T& val)
		{
			auto uni = findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			
			auto& buffer = getUniformBufferTarget(uni->ShaderType);
			buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);

			for (auto& it : m_MaterialInstances)
				it->updateMaterialValue(uni);
		}
		template<typename T>
		void Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
		{
			auto uni = findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			
			auto& buffer = getUniformBufferTarget(uni->ShaderType);
			buffer.Write((unsigned char*)&val, size, uni->Offset + offset);
			
			for (auto& it : m_MaterialInstances)
				it->updateMaterialValue(uni);
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t index = 0)
		{
			auto tex = findTexture(name);
			XYZ_ASSERT(tex, "Material texture does not exist ", name.c_str());
			
			if ((uint32_t)m_Textures.size() <= tex->Slot + index)
				m_Textures.resize((size_t)tex->Slot + 1 + index);
				
			m_Textures[size_t(tex->Slot) + size_t(index)] = texture;
		}
		void Set(const std::string& name, const Ref<Texture>& texture, uint32_t index = 0)
		{
			auto tex = findTexture(name);
			XYZ_ASSERT(tex, "Material texture does not exist ", name.c_str());

			if ((uint32_t)m_Textures.size() <= tex->Slot + index)
				m_Textures.resize((size_t)tex->Slot + 1 + index);

			m_Textures[size_t(tex->Slot) + size_t(index)] = texture;
		}

		template <typename T>
		T& Get(const std::string& name)
		{
			auto uni = findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			auto& buffer = getUniformBufferTarget(uni->ShaderType);
			return *(T*)&buffer[uni->Offset];
		}

		void Bind() const;
		void ClearTextures() { m_Textures.clear(); }
		void SetFlags(RenderFlags renderFlags) { m_Flags |= renderFlags; }
		uint64_t GetFlags() const { return m_Flags; }
	

		Ref<Shader>& GetShader() { return m_Shader; }
		const Ref<Shader>& GetShader() const { return m_Shader; }
		const Ref<ShaderAsset>& GetShaderAsset() { return m_ShaderAsset; }
		const std::vector<Ref<Texture>>& GetTextures() const { return m_Textures; }

		const uint8_t* GetVSUniformBuffer() const { return m_VSUniformBuffer; }
		const uint8_t* GetFSUniformBuffer() const { return m_FSUniformBuffer; }
		
		bool operator ==(const Material& other) const
		{
			return m_Shader->GetRendererID() == other.m_Shader->GetRendererID() && m_Flags == other.m_Flags;
		}
		bool operator != (const Material& other) const
		{
			return m_Shader->GetRendererID() != other.m_Shader->GetRendererID() || m_Flags != other.m_Flags;
		}
		//TODO TEMPORARY
		void ReloadShader() { m_Shader->Reload(); };
	private:
		void onShaderReload();
		ByteBuffer& getUniformBufferTarget(ShaderType type);
		const Uniform* findUniform(const std::string& name);
		const TextureUniform* findTexture(const std::string& name);

	private:
		Ref<Shader> m_Shader;
		Ref<ShaderAsset> m_ShaderAsset;
		std::vector<Ref<Texture>> m_Textures;

		std::unordered_set<MaterialInstance*> m_MaterialInstances;

		ByteBuffer m_VSUniformBuffer;
		ByteBuffer m_FSUniformBuffer;
		uint64_t m_Flags = 0;
	};


	class MaterialInstance : public RefCount
	{
		friend class Material;
	public:
	
		MaterialInstance(const Ref<Material>& material);
		~MaterialInstance();

		template<typename T>
		void Set(const std::string& name, const T& val)
		{
			auto uni = m_Material->findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			
			auto& buffer = getUniformBufferTarget(uni->ShaderType);
			buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			m_UpdatedValues.insert(name);
		}

		template<typename T>
		void Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
		{
			auto uni = m_Material->findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());	
			XYZ_ASSERT(size + Offset < uni->Size, "Material uniform out of range");
			
			auto& buffer = getUniformBufferTarget(uni->ShaderType);
			buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			m_UpdatedValues.insert(name);
		}

		template <typename T>
		T& Get(const std::string& name)
		{
			auto uni = m_Material->findUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			auto& buffer =getUniformBufferTarget(uni->ShaderType);
			return *(T*)&buffer[uni->Offset];
		}

		void Bind() const;

		uint64_t GetFlags() const { return m_Material->m_Flags; }	
		const uint8_t* GetVSUniformBuffer() const { return m_VSUniformBuffer; }
		const uint8_t* GetFSUniformBuffer() const { return m_FSUniformBuffer; }
		Ref<Material> GetParentMaterial() const { return m_Material; }

		static Ref<MaterialInstance> Create(const Ref<Material>& material);
	private:

		void onShaderReload();
		void updateMaterialValue(const Uniform* uni);
		ByteBuffer& getUniformBufferTarget(ShaderType type);
	private:
		Ref<Material> m_Material;

		ByteBuffer m_VSUniformBuffer;
		ByteBuffer m_FSUniformBuffer;
		std::unordered_set<std::string> m_UpdatedValues;
	};

}
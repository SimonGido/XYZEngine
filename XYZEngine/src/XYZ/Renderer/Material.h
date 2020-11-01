#pragma once
#include "Shader.h"
#include "Texture.h"
#include "RenderFlags.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/Serializable.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <queue>
#include <bitset>
#include <unordered_set>

namespace XYZ {

	class Material : public RefCount,
					 public Serializable
	{
		friend class MaterialInstance;
	public:
		Material(const Ref<Shader>& shader);
		~Material();


		template<typename T>
		void Set(const std::string& name, const T& val)
		{
			auto uni = m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			XYZ_ASSERT(uni->Offset + uni->Size <= m_Shader->GetUniformSize(), "Material uniform buffer out of range");
			

			m_Buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			for (auto& it : m_MaterialInstances)
				it->UpdateMaterialValue(uni);
		}
		template<typename T>
		void Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
		{
			auto uni = m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			XYZ_ASSERT(uni->Offset + uni->Size <= m_Shader->GetUniformSize(), "Material uniform buffer out of range");
			XYZ_ASSERT(size + offset < uni->Size, "Material uniform out of range");
	
			m_Buffer.Write((unsigned char*)&val, size, uni->Offset + offset);
			for (auto& it : m_MaterialInstances)
				it->UpdateMaterialValue(uni);
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t index = 0)
		{
			auto tex = m_Shader->FindTexture(name);
			XYZ_ASSERT(tex, "Material texture does not exist ", name.c_str());

			if ((uint32_t)m_Textures.size() <= tex->Slot + index)
				m_Textures.resize((size_t)tex->Slot + 1 + index);


			m_Textures[size_t(tex->Slot) + size_t(index)] = texture;
		}

		void SetRoutine(const std::string& name) { m_Shader->SetSubRoutine(name); }
		void SetFlags(RenderFlags renderFlags) { m_Key |= renderFlags; }
		void Bind();

		bool IsSet(RenderFlags flag) const { return ( m_Key & flag); }
		bool ContainsProperty(const std::string& name) const { return m_Shader->FindUniform(name); }
			
		int64_t GetSortKey() const { return m_Key; }
		size_t GetNumberOfTextures() const { return m_Textures.size(); }
		
		const uint8_t* GetBuffer() const { return m_Buffer; }
		const Ref<Shader>& GetShader() const { return m_Shader; }
		const std::vector<Ref<Texture>>& GetTextures() const { return m_Textures; }
		
		
		//TODO TEMPORARY
		void ReloadShader() { m_Shader->Reload(); };
	private:
		void OnShaderReload();

	private:
		Ref<Shader> m_Shader;
		std::unordered_set<MaterialInstance*> m_MaterialInstances;
		std::vector<Ref<Texture>> m_Textures;

		ByteBuffer m_Buffer;
		int64_t m_Key = 0;
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
			auto uni = m_Material->m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			XYZ_ASSERT(uni->Offset + uni->Size <= m_Material->m_Shader->GetUniformSize(), "Material uniform buffer out of range");
			m_Buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			m_UpdatedValues.insert(name);
		}

		template<typename T>
		void Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
		{
			auto uni = m_Material->m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			XYZ_ASSERT(uni->Offset + uni->Size <= m_Material->m_Shader->GetUniformSize(), "Material uniform buffer out of range");
			XYZ_ASSERT(size + Offset < uni->Size, "Material uniform out of range");
			m_Buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			m_UpdatedValues.insert(name);
		}

		template <typename T>
		T& Get(const std::string& name)
		{
			auto uni = m_Material->m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
			return *(T*)m_Buffer[uni->Offset];
		}

		void Bind();

		int64_t GetSortKey() const { return m_Material->m_Key; }
		const uint8_t* GetBuffer() const { return m_Buffer; }
		Ref<Material> GetParentMaterial() { return m_Material; }

		static Ref<MaterialInstance> Create(const Ref<Material>& material);
	private:

		void OnShaderReload();
		void UpdateMaterialValue(const Uniform* uni);

	private:
		Ref<Material> m_Material;

		ByteBuffer m_Buffer;
		std::unordered_set<std::string> m_UpdatedValues;
	};

}
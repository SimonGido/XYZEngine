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
		/**
		* Constructor ,allocates memory for the size of the shader uniforms and add reload callback to the shader,
		* and stores shared_ptr to the Shader
		* @param[in] shader 
		*/
		Material(const Ref<Shader>& shader);

		/**
		* Destructor, removes itself from MaterialManager,
		* delete storage for shader uniforms
		*/
		~Material();


		/**
		* Set the value in the buffer corresponding to the shader uniform, update value for it's instances
		* @param[in] name	Name of the uniform
		* @arg[in] val		Value for the uniform
		*/
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

		/**
		* Set the array in the buffer corresponding to the shader array uniform, update value for it's instances
		* @param[in] name	Name of the uniform
		* @arg[in] val		Value for the uniform
		* @param[in] size	Size of the val
		* @param[in] offset Offset in the shader array uniform
		*/
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

		/**
		* Set the texture corresponding to the shader texture
		* @param[in] name		Name of the texture
		* @param[in] texture	shared_ptr to the Texture2D
		* @param[in] index		Binding index of the texture
		*/
		void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t index = 0)
		{
			auto tex = m_Shader->FindTexture(name);
			XYZ_ASSERT(tex, "Material texture does not exist ", name.c_str());

			if ((uint32_t)m_Textures.size() <= tex->Slot + index)
				m_Textures.resize((size_t)tex->Slot + 1 + index);


			m_Textures[size_t(tex->Slot) + size_t(index)] = texture;
		}

		/**
		* Set routine in the shader
		* @param[in] name	Name of the routine
		*/
		void SetRoutine(const std::string& name)
		{
			m_Shader->SetSubRoutine(name);
		}

		/**
		* Set material sorting key to the render flags
		* @param[in] renderFlags	
		*/
		void SetFlags(RenderFlags renderFlags)
		{
			m_Key |= renderFlags;
		}
		

		/**
		* Bind the shader of the material, set the uniforms in the shader, bind textures
		*/
		void Bind();

		bool ContainsProperty(const std::string& name) const 
		{
			auto uni = m_Shader->FindUniform(name);
			return uni;
		}
	
		bool IsSet(RenderFlags flag) const { return ( m_Key & flag); }
		/**
		* @return sort key
		*/
		const int64_t GetSortKey() const { return m_Key; }


		const Ref<Shader>& GetShader() const{ return m_Shader; }
		

		size_t GetNumberOfTextures() const{ return m_Textures.size(); }


		const unsigned char* GetBuffer() const { return m_Buffer; }

		
		const std::vector<Ref<Texture>>& GetTextures() const { return m_Textures; }


		//TODO TEMPORARY
		void ReloadShader() { m_Shader->Reload(); };
	private:
		/**
		* It is called when shader is reloaded.
		* Reallocate storage for uniforms and call OnShaderReload for MaterialInstances
		*/
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
		/**
		* Constructor ,allocates memory for the size of the shader uniforms of the Material ,stores shared_ptr
		* to the Material and insert itself to the material instances
		* @param[in] material
		*/
		MaterialInstance(const Ref<Material>& material);

		/**
		* Destructor, deletes memory for the shader uniforms and erase itself from the Material's instances
		*/
		~MaterialInstance();


		/**
		* Set the value in the buffer corresponding to the shader uniform
		* @param[in] name	Name of the uniform
		* @arg[in] val		Value for the uniform
		*/
		template<typename T>
		void Set(const std::string& name, const T& val)
		{
			auto uni = m_Material->m_Shader->FindUniform(name);
			XYZ_ASSERT(uni, "Material uniform does not exist");
			XYZ_ASSERT(uni->Offset + uni->Size <= m_Material->m_Shader->GetUniformSize(), "Material uniform buffer out of range");
			m_Buffer.Write((unsigned char*)&val, uni->Size, uni->Offset);
			m_UpdatedValues.insert(name);
		}


		/**
		* Set the array in the buffer corresponding to the shader array uniform
		* @param[in] name	Name of the uniform
		* @arg[in] val		Value for the uniform
		* @param[in] size	Size of the val
		* @param[in] offset Offset in the shader array uniform
		*/
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

		/**
		* Set the uniforms in the shader
		*/
		void Bind();
	
		/**
		* @return shared_ptr to the Material used in constructor
		*/
		int64_t GetSortKey() const { return m_Material->m_Key; }
		Ref<Material> GetParentMaterial() { return m_Material; }

		/**
		* @param[in] material
		* @return shared_ptr to the MaterialInstance
		*/
		static Ref<MaterialInstance> Create(const Ref<Material>& material);

	private:
		/**
		* It is called when shader is reloaded.
		* Reallocate storage for uniforms
		*/
		void OnShaderReload();

		/**
		* Set uniform in the buffer if the uniform is not in set of updated values
		*/
		void UpdateMaterialValue(const Uniform* uni);

	private:
		Ref<Material> m_Material;


		ByteBuffer m_Buffer;
		std::unordered_set<std::string> m_UpdatedValues;
	};

}
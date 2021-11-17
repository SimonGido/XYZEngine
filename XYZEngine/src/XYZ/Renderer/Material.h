#pragma once
#include "Shader.h"
#include "Texture.h"
#include "XYZ/Core/Flags.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <queue>
#include <bitset>
#include <unordered_set>

namespace XYZ {

	enum class RenderFlags : uint64_t
	{
		MaterialFlag	= 1ULL << 0,
		TransparentFlag = 1ULL << 16,
		InstancedFlag	= 1ULL << 17
	};

	class Material : public Asset
	{
		friend class MaterialInstance;
	public:
		Material(const Ref<Shader>& shader);
		virtual ~Material() override;

		virtual void Bind() const;

		void Invalidate();

		template<typename T>
		void Set(const std::string& name, const T& val);
		
		template<typename T>
		void Set(const std::string& name, const T& val, uint32_t size, uint32_t offset);
		
		void SetTexture(const std::string& name, const Ref<Texture>& texture, uint32_t index = 0);

		template <typename T>
		T&	 Get(const std::string& name);
		
		bool HasProperty(const std::string& name) const;
		
		void ClearTextures()				   { m_Textures.clear(); }
		void SetFlags(RenderFlags renderFlags) { m_Flags |= renderFlags; }
		void SetRenderQueueID(uint8_t id)	   { m_RenderQueueID = id; }

		uint64_t GetFlags() const { return m_Flags.ToUlong(); }
		uint8_t  GetRenderQueueID() const { return m_RenderQueueID; }

		const Ref<Shader>& GetShader() const { return m_Shader; }
		const std::vector<Ref<Texture>>& GetTextures() const { return m_Textures; }

		const uint8_t* GetVSUniformBuffer() const { return m_VSUniformBuffer; }
		const uint8_t* GetFSUniformBuffer() const { return m_FSUniformBuffer; }
		
		bool operator ==(const Material& other) const;
		bool operator != (const Material& other) const;

		static AssetType GetStaticType() { return AssetType::Material; }
	private:
		ByteBuffer&			  getUniformBufferTarget(ShaderType type);
		const TextureUniform* findTexture(const std::string& name) const;
		std::pair<const ShaderUniform*, ShaderType> findUniform(const std::string& name) const;

	private:
		Ref<Shader>				  m_Shader;
		std::vector<Ref<Texture>> m_Textures;
		Flags<RenderFlags>		  m_Flags;

		ByteBuffer m_VSUniformBuffer;
		ByteBuffer m_FSUniformBuffer;
		uint8_t    m_RenderQueueID;
	};


	template<typename T>
	inline void Material::Set(const std::string& name, const T& val)
	{
		static_assert((!std::is_same<T, Ref<Texture>>::value) && (!std::is_same<T, Ref<Texture2D>>::value), "Use SetTexture");
		auto [uni, type] = findUniform(name);
		XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());

		auto& buffer = getUniformBufferTarget(type);
		buffer.Write((unsigned char*)&val, uni->GetSize(), uni->GetOffset());
	}

	template<typename T>
	inline void Material::Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
	{
		static_assert((!std::is_same<T, Ref<Texture>>::value) && (!std::is_same<T, Ref<Texture2D>>::value), "Use SetTexture");
		auto [uni, type] = findUniform(name);
		XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());

		auto& buffer = getUniformBufferTarget(type);
		buffer.Write((unsigned char*)&val, size, uni->GetOffset() + offset);
	}

	template<typename T>
	inline T& Material::Get(const std::string& name)
	{
		auto [uni, type] = findUniform(name);
		XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
		auto& buffer = getUniformBufferTarget(type);
		return *(T*)&buffer[uni->GetOffset()];
	}



	class MaterialInstance : public Material
	{
		friend class Material;
	public:
		MaterialInstance(const Ref<Material>& material);
		virtual ~MaterialInstance() override;

		virtual void Bind() const override;

		Ref<Material>  GetParentMaterial() const { return m_Material; }

		static Ref<MaterialInstance> Create(const Ref<Material>& material);

	private:
		Ref<Material> m_Material;
	};
}
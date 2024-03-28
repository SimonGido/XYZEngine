#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/PushConstBuffer.h"

#include <unordered_set>

namespace XYZ {
	
	class Material;
	class XYZ_API MaterialInstance : public RefCount
	{
	public:	
		virtual ~MaterialInstance() override;

		template <typename T>
		void Set(const std::string_view name, const T& value);

		template <typename T>
		T& Get(const std::string_view name);

		template <typename T>
		const T& Get(const std::string_view name) const;

		bool HasUniform(const std::string_view name) const;

		Ref<Shader>   GetShader() const;
		Ref<Material> GetMaterial() const { return m_Material; }

		PushConstBuffer GetFSUniformsBuffer() const;
		PushConstBuffer GetVSUniformsBuffer() const;

	private:
		MaterialInstance(const Ref<Material>& material);

	private:
		void initializeStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers);
		const ShaderUniform* findUniformDeclaration(const std::string_view name);
		const ShaderUniform* findUniformDeclaration(const std::string_view name) const;
	
	private:
		Ref<Material>			m_Material;
		mutable PushConstBuffer m_UniformsBuffer;

		friend class Material;
	};

	template<typename T>
	inline void MaterialInstance::Set(const std::string_view name, const T& value)
	{
		auto decl = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		if (!decl)
			return;
		m_UniformsBuffer.Write((std::byte*)&value, decl->GetSize(), decl->GetOffset());
	}
	template<typename T>
	inline T& MaterialInstance::Get(const std::string_view name)
	{
		auto decl = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		return m_UniformsBuffer.Read<T>(decl->GetOffset());
	}
	template<typename T>
	inline const T& MaterialInstance::Get(const std::string_view name) const
	{
		auto decl = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		return m_UniformsBuffer.Read<T>(decl->GetOffset());
	}
}

namespace std
{
    template<>
    struct hash<XYZ::WeakRef<XYZ::MaterialInstance>>
    {
        size_t operator()(const XYZ::WeakRef<XYZ::MaterialInstance>& obj) const
        {
			return hash<const XYZ::MaterialInstance*>()(obj.Raw());
        }
    };
}
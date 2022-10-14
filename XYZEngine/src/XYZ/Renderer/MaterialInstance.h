#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/PushConstBuffer.h"

#include <unordered_set>

namespace XYZ {
	
	class Material;
	class MaterialInstance : public RefCount
	{
	public:
		MaterialInstance(const Ref<Material>& material);
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

	protected:
		void allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers) const;
		std::pair<const ShaderUniform*, ByteBuffer*> findUniformDeclaration(const std::string_view name);
		std::pair<const ShaderUniform*, ByteBuffer*> findUniformDeclaration(const std::string_view name) const;
	
	protected:
		Ref<Material>			m_Material;
		mutable ByteBuffer		m_UniformsBuffer;

		friend class Material;
	};

	template<typename T>
	inline void MaterialInstance::Set(const std::string_view name, const T& value)
	{
		auto [decl, buffer] = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		if (!decl)
			return;
		buffer->Write((uint8_t*)&value, decl->GetSize(), decl->GetOffset());
	}
	template<typename T>
	inline T& MaterialInstance::Get(const std::string_view name)
	{
		auto [decl, buffer] = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		return buffer->Read<T>(decl->GetOffset());
	}
	template<typename T>
	inline const T& MaterialInstance::Get(const std::string_view name) const
	{
		auto [decl, buffer] = findUniformDeclaration(name);
		XYZ_ASSERT(decl != nullptr, "Could not find uniform with name");
		return buffer->Read<T>(decl->GetOffset());
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
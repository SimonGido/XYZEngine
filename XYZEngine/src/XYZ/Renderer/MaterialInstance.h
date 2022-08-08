#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Renderer/Shader.h"

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

		Ref<Shader> GetShader() const;

		const ByteBuffer GetFSUniformsBuffer() const;
		const ByteBuffer GetVSUniformsBuffer() const;

	private:
		void allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers) const;
		std::pair<const ShaderUniform*, ByteBuffer*> findUniformDeclaration(const std::string_view name);

	private:
		Ref<Material>      m_Material;
		mutable ByteBuffer m_UniformsBuffer;

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
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
	public:
		virtual ~Material() {};

		virtual void Invalidate() = 0;
		
		virtual void SetFlag(RenderFlags renderFlag, bool val = true) = 0;

		virtual void Set(const std::string& name, float value) = 0;
		virtual void Set(const std::string& name, int value) = 0;

		virtual void Set(const std::string& name, const glm::vec2& value) = 0;
		virtual void Set(const std::string& name, const glm::vec3& value) = 0;
		virtual void Set(const std::string& name, const glm::vec4& value) = 0;
		virtual void Set(const std::string& name, const glm::mat4& value) = 0;

		virtual void Set(const std::string& name, const glm::ivec2& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec3& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec4& value) = 0;

		virtual void SetImageArray(const std::string& name, Ref<Image2D> image, uint32_t arrayIndex) = 0;
		virtual void SetImage(const std::string& name, Ref<Image2D> image, int32_t mip = -1) = 0;

		virtual float&		   GetFloat(const std::string& name) = 0;
		virtual int32_t&	   GetInt(const std::string& name) = 0;
		virtual uint32_t&	   GetUInt(const std::string& name) = 0;
		virtual bool&		   GetBool(const std::string& name) = 0;

		virtual glm::ivec2&	   GetIVector2(const std::string& name) = 0;
		virtual glm::ivec3&	   GetIVector3(const std::string& name) = 0;
		virtual glm::ivec4&	   GetIVector4(const std::string& name) = 0;

		virtual glm::vec2&	   GetVector2(const std::string& name) = 0;
		virtual glm::vec3&	   GetVector3(const std::string& name) = 0;
		virtual glm::vec4&	   GetVector4(const std::string& name) = 0;
		virtual glm::mat3&	   GetMatrix3(const std::string& name) = 0;
		virtual glm::mat4&	   GetMatrix4(const std::string& name) = 0;
		virtual Ref<Texture2D> GetTexture2D(const std::string& name) = 0;
		virtual uint64_t	   GetFlags() const = 0;
		virtual uint32_t	   GetID()    const = 0;
		virtual Ref<Shader>	   GetShader()const = 0;
		
		static Ref<Material> Create(const Ref<Shader>& shader);
	};


	//template<typename T>
	//inline void Material::Set(const std::string& name, const T& val)
	//{
	//	static_assert((!std::is_same<T, Ref<Texture>>::value) && (!std::is_same<T, Ref<Texture2D>>::value), "Use SetTexture");
	//	auto [uni, type] = findUniform(name);
	//	XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
	//
	//	auto& buffer = getUniformBufferTarget(type);
	//	buffer.Write((unsigned char*)&val, uni->GetSize(), uni->GetOffset());
	//}
	//
	//template<typename T>
	//inline void Material::Set(const std::string& name, const T& val, uint32_t size, uint32_t offset)
	//{
	//	static_assert((!std::is_same<T, Ref<Texture>>::value) && (!std::is_same<T, Ref<Texture2D>>::value), "Use SetTexture");
	//	auto [uni, type] = findUniform(name);
	//	XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
	//
	//	auto& buffer = getUniformBufferTarget(type);
	//	buffer.Write((unsigned char*)&val, size, uni->GetOffset() + offset);
	//}
	//
	//template<typename T>
	//inline T& Material::Get(const std::string& name)
	//{
	//	auto [uni, type] = findUniform(name);
	//	XYZ_ASSERT(uni, "Material uniform does not exist ", name.c_str());
	//	auto& buffer = getUniformBufferTarget(type);
	//	return *(T*)&buffer[uni->GetOffset()];
	//}

}
#pragma once

#include "XYZ/Asset/Asset.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <functional>


namespace XYZ {

	/**
	* enum class represents uniform data Components
	*/
	enum class UniformDataType
	{
		None,
		Sampler2D,
		SamplerCube,
		Bool, Float, Vec2, Vec3, Vec4,
		Int, UInt, IntVec2, IntVec3, IntVec4,
		Mat3, Mat4,
		Struct,
	};

	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	static UniformDataType StringToShaderDataType(const std::string& type)
	{
		if (type == "int")			return UniformDataType::Int;
		if (type == "uint")			return UniformDataType::UInt;
		if (type == "float")		return UniformDataType::Float;
		if (type == "vec2")			return UniformDataType::Vec2;
		if (type == "vec3")			return UniformDataType::Vec3;
		if (type == "vec4")			return UniformDataType::Vec4;
		if (type == "mat3")			return UniformDataType::Mat3;
		if (type == "mat4")			return UniformDataType::Mat4;
		if (type == "sampler2D")	return UniformDataType::Sampler2D;
		return UniformDataType::None;
	}
	static uint32_t SizeOfUniformType(UniformDataType type)
	{
		switch (type)
		{
		case UniformDataType::Int:        return 4;
		case UniformDataType::UInt:       return 4;
		case UniformDataType::Float:      return 4;
		case UniformDataType::Vec2:       return 4 * 2;
		case UniformDataType::Vec3:       return 4 * 3;
		case UniformDataType::Vec4:       return 4 * 4;
		case UniformDataType::Mat3:       return 4 * 3 * 3;
		case UniformDataType::Mat4:       return 4 * 4 * 4;
		}
		return 0;
	}

	struct Uniform
	{
		std::string		Name;
		UniformDataType DataType;
		ShaderType		ShaderType;
		uint32_t		Offset;
		uint32_t		Size;
		uint32_t		Count;
		uint32_t		Location;
	};

	struct UniformList
	{
		std::vector<Uniform> Uniforms;
		uint32_t			 Size = 0;
	};

	struct TextureUniform
	{
		std::string Name;
		uint32_t Slot;
		uint32_t Count;
	};

	struct TextureUniformList
	{
		std::vector<TextureUniform> Textures;
		uint32_t Count = 0;
	};

	struct ShaderVariable
	{
		std::string     Name;
		UniformDataType	Type;
		size_t Size() const;
	};

	struct ShaderStruct
	{
		std::string					Name;
		std::vector<ShaderVariable> Variables;
		size_t Size() const;
	};

	class Shader : public Asset
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Compute(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1) const = 0;
		virtual void SetVSUniforms(ByteBuffer buffer) const = 0;
		virtual void SetFSUniforms(ByteBuffer buffer) const = 0;

		virtual void Reload() = 0;
		virtual void AddReloadCallback(std::function<void()> callback) = 0;


		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name,const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const UniformList& GetVSUniformList() const = 0;
		virtual const UniformList& GetFSUniformList() const = 0;
		virtual const TextureUniformList& GetTextureList() const = 0;

		virtual const std::string& GetPath() const = 0;
		virtual const std::string& GetName() const = 0;
		
		virtual uint32_t GetRendererID() const = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& path);
	};

	class ShaderLibrary : public RefCount
	{
	public:
		void Add(const Ref<Shader>& shader);
		//void Add(const std::string& name, const Ref<Shader>& shader);

		Ref<Shader> Load(const std::string& path);
		Ref<Shader> Load(const std::string& name, const std::string& path);
		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name);

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}
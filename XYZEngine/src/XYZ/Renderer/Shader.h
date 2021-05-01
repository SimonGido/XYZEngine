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
		Int, IntVec2, IntVec3, IntVec4,
		Mat3, Mat4,
		Struct,
	};

	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

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

	class Shader : public Asset
	{
	public:
		~Shader() = default;

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
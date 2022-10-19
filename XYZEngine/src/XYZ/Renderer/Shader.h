#pragma once


#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Renderer/ShaderResource.h"
#include "XYZ/Renderer/Buffer.h"


#include <unordered_map>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <functional>


namespace XYZ {

	/**
	* enum class represents uniform data Components
	*/
	enum class ShaderUniformDataType
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
		None,
		Vertex,
		Fragment,
		Compute
	};

	static ShaderUniformDataType StringToShaderDataType(const std::string& type)
	{
		if (type == "int")			return ShaderUniformDataType::Int;
		if (type == "uint")			return ShaderUniformDataType::UInt;
		if (type == "float")		return ShaderUniformDataType::Float;
		if (type == "vec2")			return ShaderUniformDataType::Vec2;
		if (type == "vec3")			return ShaderUniformDataType::Vec3;
		if (type == "vec4")			return ShaderUniformDataType::Vec4;
		if (type == "mat3")			return ShaderUniformDataType::Mat3;
		if (type == "mat4")			return ShaderUniformDataType::Mat4;
		if (type == "sampler2D")	return ShaderUniformDataType::Sampler2D;
		return ShaderUniformDataType::None;
	}
	static uint32_t SizeOfUniformType(ShaderUniformDataType type)
	{
		switch (type)
		{
		case ShaderUniformDataType::Int:        return 4;
		case ShaderUniformDataType::UInt:       return 4;
		case ShaderUniformDataType::Float:      return 4;
		case ShaderUniformDataType::Vec2:       return 4 * 2;
		case ShaderUniformDataType::Vec3:       return 4 * 3;
		case ShaderUniformDataType::Vec4:       return 4 * 4;
		case ShaderUniformDataType::Mat3:       return 4 * 3 * 3;
		case ShaderUniformDataType::Mat4:       return 4 * 4 * 4;
		}
		return 0;
	}

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(std::string name, ShaderUniformDataType dataType, uint32_t size, uint32_t offset, uint32_t count = 1);

		const std::string&	  GetName()		const { return m_Name; }
		ShaderUniformDataType GetDataType()	const { return m_DataType; }
		uint32_t			  GetSize()		const { return m_Size; }
		uint32_t			  GetOffset()   const { return m_Offset; }
		uint32_t			  GetCount()    const { return m_Count; }
	
	private:
		std::string			  m_Name{};
		ShaderUniformDataType m_DataType{};
		uint32_t			  m_Size{};
		uint32_t			  m_Offset{};
		uint32_t			  m_Count{};
	};


	struct ShaderBuffer
	{
		std::string Name;
		uint32_t    Size;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	struct UniformList
	{
		std::vector<ShaderUniform> Uniforms;
		uint32_t				   Size = 0;
	};

	struct TextureUniform
	{
		std::string Name;
		uint32_t	Slot;
		uint32_t	Count;
	};

	struct TextureUniformList
	{
		std::vector<TextureUniform> Textures;
		uint32_t Count = 0;
	};

	struct SpecializationCache
	{
		uint32_t Size;
		uint32_t Offset;
		uint32_t ConstantID;
		ShaderType Type;
	};

	enum class ComputeBarrierType
	{
		ShaderStorageBarrier,
		ShaderImageAccessBarrier
	};

	class Shader : public RefCount
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const {};
		virtual void Unbind() const {};
		virtual void Compute(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1, ComputeBarrierType barrierType = ComputeBarrierType::ShaderStorageBarrier) const {};
		virtual void SetVSUniforms(ByteBuffer buffer) const {};
		virtual void SetFSUniforms(ByteBuffer buffer) const {};

		virtual void Reload(bool forceCompile = false) = 0;


		virtual void SetInt(const std::string& name, int value) {};
		virtual void SetFloat(const std::string& name, float value) {};
		virtual void SetFloat2(const std::string& name,const glm::vec2& value) {};
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) {};
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) {};
		virtual void SetMat4(const std::string& name, const glm::mat4& value) {};


		
		virtual const UniformList& GetVSUniformList() const { return UniformList(); };
		virtual const UniformList& GetFSUniformList() const { return UniformList(); };
		virtual const TextureUniformList& GetTextureList() const { return TextureUniformList(); };
		virtual const std::vector<BufferLayout>& GetLayouts() const { return {}; }

		virtual const std::string& GetPath() const = 0;
		virtual const std::string& GetName() const = 0;
		virtual const std::string& GetSource() const { return std::string(); }
		
		virtual size_t   GetVertexBufferSize() const { return 0;};
		virtual uint32_t GetRendererID() const { return 0; }
		virtual size_t	 GetHash()	     const = 0;
		virtual const std::unordered_map<std::string, ShaderBuffer>& GetBuffers() const { return {}; }

		virtual const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResources() const { return {}; }
		virtual const std::unordered_map<std::string, SpecializationCache>& GetSpecializationCachce() const { return {}; }

		virtual bool IsCompiled() const { return false; };

		static Ref<Shader> Create(const std::string& path, size_t sourceHash = 0, bool forceCompile = true);
		static Ref<Shader> Create(const std::string& name, const std::string& path, size_t sourceHash = 0, bool forceCompile = true);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, size_t sourceHash = 0, bool forceCompile = true);
	};

}
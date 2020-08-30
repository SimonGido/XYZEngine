#pragma once

#include "XYZ/Core/Ref.h"

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
		NONE,
		SAMPLER2D,
		SAMPLERCUBE,
		BOOL, FLOAT, FLOAT_VEC2, FLOAT_VEC3, FLOAT_VEC4,
		INT, INT_VEC2, INT_VEC3, INT_VEC4,
		FLOAT_MAT4,
		STRUCT,
	};


	/**
	* @struct Uniform
	* @brief represents uniform of the shader
	*/
	struct Uniform
	{
		std::string Name;
		UniformDataType Type;
		uint32_t Location;
		unsigned int Offset;
		unsigned int Size;
		unsigned int Count;
		bool IsArray;
	};

	struct UniformBuffer
	{

	};

	/**
	* @struct TextureUniform
	* @brief represents texture uniform of the shader
	*/
	struct TextureUniform
	{
		std::string Name;
		uint32_t Location;
		uint32_t Slot;
		uint32_t Count;
	};

	/**
	* @struct SubRoutine
	* @brief represents sub routine of the shader
	*/
	struct SubRoutine
	{
		uint32_t ShaderType;
		uint32_t Index;
		std::string Name;
	};

	/**
	* @struct Routine
	* @brief represents routine of the shader
	*/
	struct Routine
	{
		std::vector<SubRoutine> SubRoutines;
		SubRoutine ActiveSubRoutine;
	};



	/**
	* @class Shader
	* @brief Shader encapsulates graphics API shader program.
	* Creates abstraction above shader programs
	*/
	class Shader : public RefCount
	{
	public:
		~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Compute(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1) const = 0;
		virtual void Unbind() const = 0;
		virtual void SetUniforms(unsigned char* buffer) = 0;
		virtual void SetSubRoutine(const std::string& name) = 0;
		virtual void UploadRoutines() = 0;
		virtual void Reload() = 0;
		virtual void Recompile() = 0;
		virtual void AddReloadCallback(std::function<void()> callback) = 0;
		virtual void AddSource(const std::string& filePath) = 0;


		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name,const glm::vec2& value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual std::string GetPath() const = 0;
		virtual std::string GetName() const = 0;
		virtual const Uniform* FindUniform(const std::string& name) const = 0;
		virtual const TextureUniform* FindTexture(const std::string& name) const = 0;

		virtual uint32_t GetUniformSize() const = 0;
		virtual const std::vector<Uniform>& GetUniforms() const = 0;

		/**
		* Create Shader
		* @param[in] path	File path to the glsl code
		* @return shared_ptr to Shader
		*/
		static Ref<Shader> Create(const std::string& path);

		/**
		* Create Shader
		* @param[in] name	Name of the shader
		* @param[in] path	File path to the glsl code
		* @return shared_ptr to Shader
		*/
		static Ref<Shader> Create(const std::string& name, const std::string& path);

	protected:
		enum class ShaderProgramType
		{
			RENDER,
			COMPUTE
		};
	};


	class ShaderLibrary
	{
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	public:
		void Add(const Ref<Shader>& shader);
		//void Add(const std::string& name, const Ref<Shader>& shader);

		Ref<Shader> Load(const std::string& path);
		Ref<Shader> Load(const std::string& name, const std::string& path);
		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name);
	};

}
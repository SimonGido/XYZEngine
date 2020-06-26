#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <functional>


namespace XYZ {

	/**
	* enum class represents uniform data types
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
		std::string name;
		UniformDataType type;
		uint32_t location;
		unsigned int offset;
		unsigned int size;
		unsigned int count;
		bool isArray;
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
		std::string name;
		uint32_t location;
		unsigned int slot;
		unsigned int count;
	};

	/**
	* @struct SubRoutine
	* @brief represents sub routine of the shader
	*/
	struct SubRoutine
	{
		unsigned int shaderType;
		unsigned int index;
		std::string name;
	};

	/**
	* @struct Routine
	* @brief represents routine of the shader
	*/
	struct Routine
	{
		std::vector<SubRoutine> subRoutines;
		SubRoutine activeSubRoutine;
	};



	/**
	* @class Shader
	* @brief Shader encapsulates graphics API shader program.
	* Creates abstraction above shader programs
	*/
	class Shader
	{
	public:
		~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Compute(unsigned int groupX, unsigned int groupY = 1, unsigned int groupZ = 1) const = 0;
		virtual void Unbind() const = 0;
		virtual void SetUniforms(unsigned char* buffer) = 0;
		virtual void SetSubRoutine(const std::string& name) = 0;
		virtual void UploadRoutines() = 0;
		virtual void Reload() = 0;
		virtual void Recompile() = 0;
		virtual void AddReloadCallback(std::function<void()> callback) = 0;
		virtual void AddSource(const std::string& filePath) = 0;


		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual std::string GetPath() const = 0;
		virtual std::string GetName() const = 0;
		virtual const Uniform* FindUniform(const std::string& name) = 0;
		virtual const TextureUniform* FindTexture(const std::string& name) = 0;

		virtual unsigned int GetUniformSize() = 0;


		/**
		* Create Shader
		* @param[in] path	File path to the glsl code
		* @return shared_ptr to Shader
		*/
		static std::shared_ptr<Shader> Create(const std::string& path);

		/**
		* Create Shader
		* @param[in] name	Name of the shader
		* @param[in] path	File path to the glsl code
		* @return shared_ptr to Shader
		*/
		static std::shared_ptr<Shader> Create(const std::string& name, const std::string& path);

	protected:
		enum class ShaderProgramType
		{
			RENDER,
			COMPUTE
		};
	};


	class ShaderLibrary
	{
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	public:
		void Add(const std::shared_ptr<Shader>& shader);
		//void Add(const std::string& name, const Ref<Shader>& shader);

		std::shared_ptr<Shader> Load(const std::string& path);
		std::shared_ptr<Shader> Load(const std::string& name, const std::string& path);
		std::shared_ptr<Shader> Get(const std::string& name);
		bool Exists(const std::string& name);
	};

}
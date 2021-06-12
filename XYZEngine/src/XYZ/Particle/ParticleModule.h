#pragma once

#include "XYZ/Utils/StringUtils.h"

#include <glm/glm.hpp>

#include <stdint.h>

namespace XYZ {
	
	
	enum class ParticleShaderDataType
	{
		None, Int, UInt, Float, Float2, Float3, Float4
	};
	
	inline ParticleShaderDataType StringToShaderDataType(const std::string& str)
	{
		if (str == "int")
			return ParticleShaderDataType::Int;
		if (str == "uint")
			return ParticleShaderDataType::UInt;
		if (str == "float")
			return ParticleShaderDataType::Float;
		if (str == "vec2")
			return ParticleShaderDataType::Float2;
		if (str == "vec3")
			return ParticleShaderDataType::Float3;
		if (str == "vec4")
			return ParticleShaderDataType::Float4;

		XYZ_ASSERT(false, "Invalid type");
		return ParticleShaderDataType::None;
	}
	
	struct ParticleModuleElement
	{
		std::string			   Name;
		ParticleShaderDataType Type;
	};

	template <size_t ElementCount>
	struct ParticleModule
	{
		ParticleModule()
			: 
			Enabled(false), 
			m_ID(0)
		{}

		std::string				Name;
		std::string				Source;
		ParticleModuleElement	Elements[ElementCount];
		bool					Enabled;

		void LoadFromFile(const std::string& filepath);
		
		std::string ElementsToStruct() const;

		uint32_t GetID() const { return m_ID; }
	private:
		void parse(const std::string& source);
		void parseStruct(const std::string& structSource);

	private:
		uint32_t m_ID;
	};


	template<size_t ElementCount>
	inline void ParticleModule<ElementCount>::LoadFromFile(const std::string& filepath)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			Source.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&Source[0], Source.size());
			in.close();
			parse(Source);
		}
	}

	template<size_t ElementCount>
	inline std::string ParticleModule<ElementCount>::ElementsToStruct() const
	{
		std::string result;
		result.append("struct " + Name + "\n{\n");
		for (size_t i = 0; i < ElementsCount; ++i)
		{
			switch (Elements[i].Type)
			{
			case ParticleShaderDataType::Int:
				result.append("int ");
				break;
			case ParticleShaderDataType::UInt:
				result.append("uint ");
				break;
			case ParticleShaderDataType::Float:
				result.append("float ");
				break;
			case ParticleShaderDataType::Float2:
				result.append("vec2 ");
				break;
			case ParticleShaderDataType::Float3:
				result.append("vec3 ");
				break;
			case ParticleShaderDataType::Float4:
				result.append("vec4 ");
				break;
			}
			result.append(Elements[i].Name + ";\n");
		}
		result.append("\n};\n");
		return result;
	}

	template<size_t ElementCount>
	inline void ParticleModule<ElementCount>::parse(const std::string& source)
	{
		const char* id = Utils::FindToken(source.c_str(), "#id");
		if (id)
		{
			std::vector<std::string> splitSource = Utils::SplitString(id, " \n");
			m_ID = atoi(splitSource[1].c_str());
		}
		else
		{
			XYZ_ASSERT(false, "Failed to find id");
		}
		const char* token = Utils::FindToken(source.c_str(), "struct");
		if (token)
		{ 
			parseStruct(token);
		}
		else
		{
			XYZ_ASSERT(false, "Failed to find struct");
		}
	}
	template<size_t ElementCount>
	inline void ParticleModule<ElementCount>::parseStruct(const std::string& structSource)
	{
		std::vector<std::string> tokens = std::move(Utils::SplitString(structSource, "\t\n"));
		std::vector<std::string> structName = std::move(Utils::SplitString(tokens[0], " \r"));
		Name = structName[1];
		size_t varCount = 0;
		for (size_t i = 1; i < tokens.size(); ++i)
		{
			std::vector<std::string> variables = std::move(Utils::SplitString(tokens[i], " \r"));
			if (variables.size() > 1)
			{
				XYZ_ASSERT(varCount < ElementCount, "Number of elements in module is bigger than ElementCount");
				Elements[varCount].Type = StringToShaderDataType(variables[0]);
				variables[1].pop_back(); // pop ;
				Elements[varCount].Name = variables[1];
				varCount++;
			}
		}
	}
}
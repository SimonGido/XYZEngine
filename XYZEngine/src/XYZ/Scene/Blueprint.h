#pragma once
#include "XYZ/Asset/Asset.h"
#include "BlueprintVariableType.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct BlueprintVariable
	{
		std::string  Name;
		VariableType Type;
		uint32_t     Size = 0; // only when adding structs
		bool		 IsArray = false;
	};

	struct BlueprintInputArgument
	{
		VariableType Type;
		std::string  Name;
		bool		 Output = false;
	};

	struct BlueprintFunction
	{
		std::string Name;
		std::string SourceCode;

		std::vector<BlueprintInputArgument> Arguments;
		bool								Entry = false;
	};

	struct BlueprintFunctionCall
	{
		std::string		  Name;
		std::vector<BlueprintInputArgument> Arguments;
	};

	struct BlueprintStruct
	{
		std::string Name;
		std::vector<BlueprintVariable> Variables;
	};

	struct BlueprintFunctionSequence
	{
		BlueprintFunction				   EntryPoint;
		std::vector<BlueprintFunctionCall> FunctionCalls;
	};

	enum class BlueprintBufferType
	{
		Uniform, Storage
	};

	enum class BlueprintBufferLayout
	{
		STD140,
		STD430
	};

	static std::string BufferTypeToString(BlueprintBufferType type)
	{
		switch (type)
		{
		case XYZ::BlueprintBufferType::Uniform:
			return "uniform";
		case XYZ::BlueprintBufferType::Storage:
			return "buffer";
		}
		XYZ_ASSERT(false, "");
		return "";
	}

	static std::string BufferLayoutToString(BlueprintBufferLayout layout)
	{
		switch (layout)
		{
		case XYZ::BlueprintBufferLayout::STD140:
			return "std140";
		case XYZ::BlueprintBufferLayout::STD430:
			return "std430";
		}
		return "";
	}

	struct BlueprintBuffer
	{
		std::string Name;
		uint32_t	Binding;
		uint32_t    Set = 0;

		BlueprintBufferType Type;
		BlueprintBufferLayout LayoutType;

		std::vector<BlueprintVariable> Variables;
	};

	class Blueprint : public Asset
	{
	public:
		Blueprint();

		virtual AssetType GetAssetType() const override { return AssetType::Blueprint; }

		void AddStruct(const BlueprintStruct& str);

		void AddFunction(const BlueprintFunction& func);

		void AddBuffer(const BlueprintBuffer& buffer);

		void SetFunctionSequence(const BlueprintFunctionSequence& seq);

		void SetWorkgroups(const glm::ivec3& workgroups);

		void Rebuild();

		const std::string GetSourceCode() const { return m_SourceCode; }

		static AssetType GetStaticType() { return AssetType::Blueprint; }

	private:
		void addBufferDefinition(const BlueprintBuffer& buffer);
		void addStructDefinition(const BlueprintStruct& str);
		void addFunctionDefinition(const BlueprintFunction& func);
		void addFunctionSequence(const BlueprintFunctionSequence& seq);
		void addFunctionCall(const BlueprintFunctionCall& call);
		void addBeginFunction(const BlueprintFunction& func);
		void addMain(const BlueprintFunctionSequence& seq);

	private:
		std::vector<BlueprintStruct>   m_Structs;
		std::vector<BlueprintFunction> m_Functions;
		std::vector<BlueprintBuffer>   m_Buffers;
		glm::ivec3					   m_Workgroups;
		BlueprintFunctionSequence	   m_Sequence;
		std::string				       m_SourceCode;
		bool						   m_BuildRequired = false;
	};
}
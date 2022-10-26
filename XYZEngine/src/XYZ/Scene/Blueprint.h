#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Utils/VariableTypes.h"

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
		VariableType					    OutputType = VariableType::Void;
	};

	struct BlueprintFunctionCall
	{
		std::string		  Name;
		VariableType	  OutputType;
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

	class Blueprint : public Asset
	{
	public:
		virtual AssetType GetAssetType() const override { return AssetType::Blueprint; }

		void AddStruct(const BlueprintStruct& str);

		void AddFunction(const BlueprintFunction& func);

		void SetFunctionSequence(const BlueprintFunctionSequence& seq);

		void Rebuild();

		const std::string GetSourceCode() const { return m_SourceCode; }

		static AssetType GetStaticType() { return AssetType::Blueprint; }

	private:
		void addStructDefinition(const BlueprintStruct& str);
		void addFunctionDefinition(const BlueprintFunction& func);
		void addFunctionSequence(const BlueprintFunctionSequence& seq);
		void addFunctionCall(const BlueprintFunctionCall& call);
		void addBeginFunction(const BlueprintFunction& func);

	private:
		std::vector<BlueprintStruct>   m_Structs;
		std::vector<BlueprintFunction> m_Functions;
		BlueprintFunctionSequence	   m_Sequence;
		std::string				       m_SourceCode;
		bool						   m_BuildRequired = false;
	};
}
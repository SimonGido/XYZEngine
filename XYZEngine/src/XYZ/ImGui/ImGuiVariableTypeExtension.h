#pragma once
#include "XYZ/Scene/BlueprintVariableType.h"

namespace XYZ {

	class XYZ_API ImGuiVariableTypeExtension
	{
		using EditFunction = std::function<bool(const char* id, std::byte* data)>;
	public:
		ImGuiVariableTypeExtension();

		void RegisterEditFunction(const std::string& name, EditFunction func);

		bool EditValue(const std::string& name, const char*id, std::byte* data);

		bool HasEdit(const std::string& name) const;

	private:
		std::unordered_map<std::string, EditFunction> m_EditFunctions;
	};

}
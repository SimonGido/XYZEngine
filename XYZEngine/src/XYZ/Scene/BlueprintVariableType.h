#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Core/Assert.h"

#include <string>

namespace XYZ {
	
	struct VariableType
	{
		bool operator==(const VariableType& other) const
		{
			return Name == other.Name;
		}
		std::string Name;
		uint32_t	Size;
	};

	class VariableManager
	{
	public:
		VariableManager();

		void RegisterVariable(const std::string& name, uint32_t size);

		const std::unordered_map<std::string, VariableType>& GetVariableTypes() const { return m_Variables; }

		const VariableType& GetVariable(const std::string& name) const { return m_Variables.at(name); }
	private:
		std::unordered_map<std::string, VariableType> m_Variables;
	};
}
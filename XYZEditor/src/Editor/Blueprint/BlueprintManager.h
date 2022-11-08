#pragma once
#include "XYZ/Scene/BlueprintVariableType.h"
#include "XYZ/Scene/Blueprint.h"

#include <string>

namespace XYZ {
	namespace Editor {


		// TODO: rename to registry, move it to the XYZEngine
		class BlueprintManager
		{
		public:
			BlueprintManager(VariableManager* manager);



			BlueprintFunction* FindFunction(const std::string_view name);
			const BlueprintFunction* FindFunction(const std::string_view name) const;

			std::vector<BlueprintStruct>&		GetStructTypes() { return m_Types; }
			const std::vector<BlueprintStruct>& GetStructTypes() const { return m_Types; }

			std::vector<BlueprintFunction>&		  GetFunctions() { return m_Functions; }
			const std::vector<BlueprintFunction>& GetFunctions() const { return m_Functions; }


			
		private:
			std::vector<BlueprintStruct>   m_Types;
			std::vector<BlueprintFunction> m_Functions;
			VariableManager*			   m_VariableManager;
		};

	}
}
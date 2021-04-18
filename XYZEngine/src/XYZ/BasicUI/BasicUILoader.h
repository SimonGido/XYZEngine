#pragma once
#include "BasicUI.h"

#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {

	class bUILoader
	{
	public:
		static void Load(const char* filepath);


	private:
		static void findSize(size_t& size, const YAML::Node& data);
		static void loadUIElements(bUIElement* parent, const glm::vec2& aspect, const YAML::Node& data);

		static bUIElement* createElement(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
	};
}
#pragma once
#include "BasicUI.h"

#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {

	class bUILoader
	{
	public:
		static bool Load(const std::string& filepath, bool scale = false);
		static void Save(const std::string& name, const char* filepath);

	private:
		static void findSize(size_t& size, const YAML::Node& data);
		static void loadUIElements(bUIAllocator* allocator, bUIElement* parent, const glm::vec2& aspect, const YAML::Node& data);
		
		static bUIElement* createElement(
			bUIAllocator* allocator, 
			bUIElement* parent,
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
	};
}
#include "stdafx.h"
#include "BasicUILoader.h"

#include "XYZ/Core/Application.h"


namespace XYZ {
	namespace Helper {
		size_t TypeToSize(bUIElementType type)
		{
			switch (type)
			{
			case XYZ::bUIElementType::Button:
				return sizeof(bUIButton);
			case XYZ::bUIElementType::Checkbox:
				return sizeof(bUICheckbox);
			case XYZ::bUIElementType::Slider:
				return sizeof(bUISlider);
			case XYZ::bUIElementType::Group:
				return sizeof(bUIGroup);
			default:
				return 0;
			}
		}
	}
	void bUILoader::Load(const char* filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto& app = Application::Get();
		
		glm::vec2 size = data["Size"].as<glm::vec2>();
		glm::vec2 aspect = size / glm::vec2(app.GetWindow().GetWidth(), 
											app.GetWindow().GetHeight());
		
		bUIElement* parent = nullptr;
		auto elements = data["bUIElements"];
		size_t dataSize = 0;
		findSize(dataSize, elements);
		bUI::getContext().Data.Reserve(dataSize);
		loadUIElements(nullptr, aspect, elements);
		for (size_t i = 0; i < bUI::getContext().Data.Size(); ++i)
		{
			bUIElement* element = bUI::getContext().Data.GetElement<bUIElement>(i);
			bUI::getContext().ElementMap[element->Name] = element;
		}
	}

	void bUILoader::findSize(size_t& size, const YAML::Node& data)
	{
		for (auto& element : data)
		{
			bUIElementType type = (bUIElementType)element["Type"].as<uint32_t>();
			size += Helper::TypeToSize(type);
			auto children = element["bUIElements"];
			if (children)
				findSize(size, children);
		}
	}
	void bUILoader::loadUIElements(bUIElement* parent, const glm::vec2& aspect, const YAML::Node& data)
	{
		for (auto &element : data)
		{
			glm::vec2 coords		 = element["Coords"].as<glm::vec2>();
			glm::vec2 size			 = element["Size"].as<glm::vec2>();
			glm::vec4 color			 = element["Color"].as<glm::vec4>();
			std::string label		 = element["Label"].as<std::string>();
			std::string name		 = element["Name"].as<std::string>();
			bUIElementType type		 = (bUIElementType)element["Type"].as<uint32_t>();		
			bUIElement* newParent	 = createElement(coords / aspect, size / aspect, color, label, name, type);
			newParent->Parent		 = parent;
			auto children = element["bUIElements"];
			if (children)
				loadUIElements(newParent, aspect, children);
		}
	}
	bUIElement* bUILoader::createElement(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
	{
		bUIElement* element = nullptr;
		switch (type)
		{
		case XYZ::bUIElementType::Button:
			element = bUI::getContext().Data.CreateElement<bUIButton>(coords, size, color,  label, name, type);
			break;
		case XYZ::bUIElementType::Checkbox:
			element = bUI::getContext().Data.CreateElement<bUICheckbox>(coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Slider:
			element = bUI::getContext().Data.CreateElement<bUISlider>(coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Group:
			element = bUI::getContext().Data.CreateElement<bUIGroup>(coords, size, color, label, name, type);
			break;
		default:
			break;
		}
		return element;
	}
}
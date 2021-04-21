#include "stdafx.h"
#include "BasicUILoader.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Utils/StringUtils.h"


namespace XYZ {
	namespace Helper {
		static size_t TypeToSize(bUIElementType type)
		{
			switch (type)
			{
			case XYZ::bUIElementType::Button:
				return sizeof(bUIButton);
			case XYZ::bUIElementType::Checkbox:
				return sizeof(bUICheckbox);
			case XYZ::bUIElementType::Slider:
				return sizeof(bUISlider);
			case XYZ::bUIElementType::Window:
				return sizeof(bUIWindow);
			case XYZ::bUIElementType::Scrollbox:
				return sizeof(bUIScrollbox);
			case XYZ::bUIElementType::Tree:
				return sizeof(bUITree);
			case XYZ::bUIElementType::Float:
				return sizeof(bUIFloat);
			default:
				return 0;
			}
		}
		static bUIElementType StringToType(const std::string& str)
		{
			if (str == "Button")
				return bUIElementType::Button;
			if (str == "Checkbox")
				return bUIElementType::Checkbox;
			if (str == "Slider")
				return bUIElementType::Slider;
			if (str == "Window")
				return bUIElementType::Window;
			if (str == "Scrollbox")
				return bUIElementType::Scrollbox;
			if (str == "Tree")
				return bUIElementType::Tree;
			if (str == "Float")
				return bUIElementType::Float;
			
			XYZ_ASSERT(false, "Invalid type");
			return bUIElementType::None;
		}
		static std::string TypeToString(bUIElementType type)
		{
			switch (type)
			{
			case XYZ::bUIElementType::Button:
				return "Button";
			case XYZ::bUIElementType::Checkbox:
				return "Checkbox";
			case XYZ::bUIElementType::Slider:
				return "Slider";
			case XYZ::bUIElementType::Window:
				return "Window";
			case XYZ::bUIElementType::Scrollbox:
				return "Scrollbox";
			case XYZ::bUIElementType::Tree:
				return "Tree";
			case XYZ::bUIElementType::Float:
				return "Float";
			}
			XYZ_ASSERT(false, "Type is none");
			return "None";
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
		
		std::string name = Utils::GetFilenameWithoutExtension(filepath);

		bUIAllocator* allocator = nullptr;
		if (bUI::getContext().Data.Exist(name))
			allocator = &bUI::getContext().Data.GetAllocator(name);
		else
			allocator = &bUI::getContext().Data.CreateAllocator(name, dataSize);

		allocator->Reserve(dataSize);
		loadUIElements(allocator, nullptr, aspect, elements);
	}

	void bUILoader::Save(const std::string& name, const char* filepath)
	{
		YAML::Emitter out;

		bUIAllocator& allocator = bUI::getContext().Data.GetAllocator(name);
		bUIElement* parent = nullptr;

		out << YAML::BeginMap;	
		out << YAML::Key << "Size" << bUI::getContext().ViewportSize;
		out << YAML::Key << "bUIElements";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < allocator.Size() - 1; ++i)
		{
			bUIElement* element = allocator.GetElement<bUIElement>(i);
			bUIElement* next = allocator.GetElement<bUIElement>(i + 1);

			uint32_t depth = element->depth();
			uint32_t nextDepth = next->depth();
			

			out << YAML::BeginMap;
			out << YAML::Key << "Coords" << YAML::Value << element->Coords;
			out << YAML::Key << "Size"   << YAML::Value << element->Size;
			out << YAML::Key << "Color"  << element->Color;
			out << YAML::Key << "Label"  << YAML::Value << element->Label;
			out << YAML::Key << "Name"   << YAML::Value << element->Name;
			out << YAML::Key << "Type"   << YAML::Value << Helper::TypeToString(element->Type);

			if (depth < nextDepth)
			{
				out << YAML::Key << "bUIElements";
				out << YAML::Value << YAML::BeginSeq;
			}
			else if (depth > nextDepth)
			{
				uint32_t diff = depth - nextDepth;
				for (uint32_t i = 0; i < diff; ++i)
				{
					out << YAML::EndMap;
					out << YAML::EndSeq;
				}
				out << YAML::EndMap;
			}
			else
			{
				out << YAML::EndMap;
			}
		}
		bUIElement* element = allocator.GetElement<bUIElement>(allocator.Size() - 1);
		out << YAML::BeginMap;
		out << YAML::Key << "Coords" << element->Coords;
		out << YAML::Key << "Size"   << element->Size;
		out << YAML::Key << "Color"  << element->Color;
		out << YAML::Key << "Label"  << element->Label;
		out << YAML::Key << "Name"   << element->Name;
		out << YAML::Key << "Type"   << YAML::Value << Helper::TypeToString(element->Type);
		out << YAML::EndMap;

		out << YAML::EndSeq;
		out << YAML::EndMap;


		std::ofstream fout(filepath);
		fout << out.c_str();	
	}

	void bUILoader::findSize(size_t& size, const YAML::Node& data)
	{
		for (auto& element : data)
		{
			bUIElementType type = Helper::StringToType(element["Type"].as<std::string>());
			size += Helper::TypeToSize(type);
			auto children = element["bUIElements"];
			if (children)
				findSize(size, children);
		}
	}
	void bUILoader::loadUIElements(bUIAllocator* allocator, bUIElement* parent, const glm::vec2& aspect, const YAML::Node& data)
	{
		for (auto &element : data)
		{
			glm::vec2 coords		 = element["Coords"].as<glm::vec2>();
			glm::vec2 size			 = element["Size"].as<glm::vec2>();
			glm::vec4 color			 = element["Color"].as<glm::vec4>();
			std::string label		 = element["Label"].as<std::string>();
			std::string name		 = element["Name"].as<std::string>();
			bUIElementType type		 = Helper::StringToType(element["Type"].as<std::string>());		
			bUIElement* newParent	 = createElement(allocator, parent, coords / aspect, size / aspect, color, label, name, type);
			newParent->Parent		 = parent;
			auto children = element["bUIElements"];
			if (children)
				loadUIElements(allocator, newParent, aspect, children);
		}
	}
	
	bUIElement* bUILoader::createElement(bUIAllocator* allocator, bUIElement* parent, const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
	{
		bUIElement* element = nullptr;
		switch (type)
		{
		case XYZ::bUIElementType::Button:
			element = allocator->CreateElement<bUIButton>(parent, coords, size, color,  label, name, type);
			break;
		case XYZ::bUIElementType::Checkbox:
			element = allocator->CreateElement<bUICheckbox>(parent,coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Slider:
			element = allocator->CreateElement<bUISlider>(parent,coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Window:
			element = allocator->CreateElement<bUIWindow>(parent, coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Scrollbox:
			element = allocator->CreateElement<bUIScrollbox>(parent, coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Tree:
			element = allocator->CreateElement<bUITree>(parent, coords, size, color, label, name, type);
			break;
		case XYZ::bUIElementType::Float:
			element = allocator->CreateElement<bUIFloat>(parent, coords, size, color, label, name, type);
			break;
		default:
			XYZ_ASSERT(false, "");
			break;
		}
		return element;
	}
}
#include "stdafx.h"
#include "InGuiSerializer.h"

#include "XYZ/Utils/YamlUtils.h"

#include <stack>

namespace XYZ {
	void InGuiSerializer::Serialize(const InGuiContext& context, const std::string& filepath)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		// Windows
		out << YAML::Key << "Windows";
		out << YAML::Value << YAML::BeginSeq;
		for (auto window : context.m_Windows)
		{
			if (!window->IsInitialized)
				continue;
			bool collapsed = IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed);
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << window->Name;
			out << YAML::Key << "Position" << YAML::Value << window->Position;
			out << YAML::Key << "Size" << YAML::Value << window->Size;
			out << YAML::Key << "OriginalSize" << YAML::Value << window->OriginalSize;
			out << YAML::Key << "Collapsed" << YAML::Value << collapsed;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		if (context.m_DockSpace.IsInitialized())
		{
			// DockSpace
			out << YAML::Key << "DockSpace";
			out << YAML::Value << YAML::BeginSeq;
	
			std::stack<InGuiDockNode*> nodes;
			nodes.push(context.m_DockSpace.Root);
			while (!nodes.empty())
			{
				InGuiDockNode* tmp = nodes.top();
				nodes.pop();
				if (tmp->Split != SplitType::None)
				{
					nodes.push(tmp->Children[0]);
					nodes.push(tmp->Children[1]);
				}
				out << YAML::BeginMap;

				out << YAML::Key << "ID" << YAML::Value << tmp->ID;
				out << YAML::Key << "Position" << YAML::Value << tmp->Position;
				out << YAML::Key << "Size" << YAML::Value << tmp->Size;
				out << YAML::Key << "Split" << YAML::Value << (uint32_t)tmp->Split;
				if (tmp->Split != SplitType::None)
				{
					out << YAML::Key << "Child0" << YAML::Value << tmp->Children[0]->ID;
					out << YAML::Key << "Child1" << YAML::Value << tmp->Children[1]->ID;
				}

				out << YAML::Key << "Windows";
				out << YAML::Value << YAML::BeginSeq;
				for (auto window : tmp->DockedWindows)
				{
					out << window->Name;
				}
				out << YAML::Value << YAML::EndSeq;
				out << YAML::EndMap;
			}
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void InGuiSerializer::Deserialize(InGuiContext& context, const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());
		auto windows = data["Windows"];
		for (auto& window : windows)
		{
			InGuiWindow* win  = context.CreateInGuiWindow(window["Name"].as<std::string>().c_str());
			win->Position     = window["Position"].as<glm::vec2>();
			win->Size         = window["Size"].as<glm::vec2>();
			win->OriginalSize = window["OriginalSize"].as<glm::vec2>();
			bool collapsed    = window["Collapsed"].as<bool>();
			if (collapsed)
				win->EditFlags |= InGuiWindowEditFlags::Collapsed;			
		}

		InGuiID maxNodeID = 0;
		std::unordered_map<InGuiID, InGuiDockNode*> dockNodeMap;
		auto dockSpace = data["DockSpace"];
		if (dockSpace)
		{
			for (auto& node : dockSpace)
			{
				InGuiDockNode* newNode   = context.m_DockSpace.Pool.Allocate<InGuiDockNode>(node["ID"].as<size_t>());
				newNode->Position		 = node["Position"].as<glm::vec2>();
				newNode->Size			 = node["Size"].as<glm::vec2>();
				newNode->Split			 = (SplitType)node["Split"].as<uint32_t>();		
				auto dockedWindows		 = node["Windows"];
				for (auto window : dockedWindows)
				{
					InGuiWindow* win = context.GetInGuiWindow(window.as<std::string>().c_str());
					newNode->DockedWindows.push_back(win);
					win->DockNode = newNode;
				}

				maxNodeID = std::max(newNode->ID, maxNodeID);
				dockNodeMap[newNode->ID] = newNode;
			}
			for (auto& node : dockSpace)
			{
				auto child0 = node["Child0"];
				auto child1 = node["Child1"];
				if (child0 && child1)
				{
					InGuiDockNode* parent = dockNodeMap.find(node["ID"].as<size_t>())->second;
					InGuiDockNode* firstChild = dockNodeMap.find(child0.as<size_t>())->second;
					InGuiDockNode* secondChild = dockNodeMap.find(child1.as<size_t>())->second;

					parent->Children[0] = firstChild;
					parent->Children[1] = secondChild;
					firstChild->Parent = parent;
					secondChild->Parent = parent;
				}
			}
			context.m_DockSpace.Root = dockNodeMap[0];
			context.m_DockSpace.NextID = maxNodeID;
		}
	}
}
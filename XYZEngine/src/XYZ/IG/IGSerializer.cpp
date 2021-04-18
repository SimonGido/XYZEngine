#include "stdafx.h"
#include "IGSerializer.h"
#include "IGUIElements.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

static YAML::Emitter& ToVec2(YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

static YAML::Emitter& ToVec3(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

static YAML::Emitter& ToVec4(YAML::Emitter& out, const glm::vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

namespace XYZ {

	static void SerializeDockNode(IGDockNode* node, const std::unordered_map<size_t, uint32_t>& poolMap, YAML::Emitter& out)
	{
		if (node)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "ID" << YAML::Value << node->ID;
			out << YAML::Key << "Position" << YAML::Value;
			ToVec2(out, node->Data.Position);
			out << YAML::Key << "Size" << YAML::Value;
			ToVec2(out, node->Data.Size);
			out << YAML::Key << "Type" << YAML::Value << ToUnderlying(node->Type);
			out << YAML::Key << "Windows" << YAML::Value << YAML::BeginSeq;
			for (auto win : node->Data.Windows)
			{
				std::hash<const IGWindow*> hasher;
				auto& it = poolMap.find(hasher(win));
				out << YAML::BeginMap;
				out << YAML::Key << "HierarchyIndex" << YAML::Value << win->GetID();
				out << YAML::Key << "PoolIndex" << YAML::Value << it->second;	
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			if (node->Parent)
				out << YAML::Key << "Parent" << YAML::Value << node->Parent->ID;
			if (node->FirstChild)
				out << YAML::Key << "FirstChild" << YAML::Value << node->FirstChild->ID;
			if (node->SecondChild)
				out << YAML::Key << "SecondChild" << YAML::Value << node->SecondChild->ID;
			out << YAML::EndMap; // Window
			SerializeDockNode(node->FirstChild, poolMap, out);
			SerializeDockNode(node->SecondChild, poolMap, out);
		}
	}
	void IGSerializer::Serialize(const char* filepath, const IGContext& context)
	{
		YAML::Emitter out;
		std::ofstream fout(filepath);

		out << YAML::BeginMap;
		out << YAML::Key << "IG";
		out << YAML::Value << "IG";

		out << YAML::Key << "Root Elements";
		out << YAML::Value << YAML::BeginSeq;
		uint32_t counter = 0;
		std::unordered_map<size_t, uint32_t> poolMap;
		for (auto& pool : context.Allocator.GetPools())
		{
			for (auto root : pool.GetRootElementIDs())
			{
				const IGWindow* window = static_cast<const IGWindow*>(pool.GetHierarchy().GetData(root));

				std::hash<const IGWindow*> hasher;
				poolMap[hasher(window)] = counter;

				out << YAML::BeginMap;
				out << YAML::Key << "Label" << YAML::Value << window->Label;
				out << YAML::Key << "Position" << YAML::Value;
				ToVec2(out, window->Position);
				out << YAML::Key << "Size" << YAML::Value;
				ToVec2(out, window->Size);
				out << YAML::Key << "Color" << YAML::Value;
				ToVec4(out, window->Color);
				out << YAML::Key << "FrameColor" << YAML::Value;
				ToVec4(out, window->FrameColor);
				out << YAML::Key << "Flags" << YAML::Value << (uint32_t)window->Flags;
				out << YAML::Key << "ID" << YAML::Value << window->GetID();
				out << YAML::Key << "Pool" << YAML::Value << counter;
				out << YAML::EndMap; // Window
			}
			counter++;
		}
		out << YAML::EndSeq;
		out << YAML::Key << "Dockspace";
		out << YAML::Value << YAML::BeginSeq;
		SerializeDockNode(context.Dockspace.m_Root, poolMap, out);
		out << YAML::EndSeq;

		out << YAML::EndMap;
		fout << out.c_str();
	}

	void IGSerializer::Deserialize(const char* filepath, IGContext& context)
	{
		std::ifstream stream(filepath);
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
			auto& roots = data["Root Elements"];
			if (roots)
			{
				for (auto& it : roots)
				{
					int32_t id				= it["ID"].as<int32_t>();		
					size_t  poolIndex		= it["Pool"].as<size_t>();
					IGPool& pool			= context.Allocator.GetPools()[poolIndex];
					IGWindow* window		= static_cast<IGWindow*>(pool.GetHierarchy().GetData(id));
					window->Label			= it["Label"].as<std::string>();
					window->Position		= it["Position"].as<glm::vec2>();
					window->Size			= it["Size"].as<glm::vec2>();
					window->Color			= it["Color"].as<glm::vec4>();
					window->FrameColor		= it["FrameColor"].as<glm::vec4>();
					window->Flags			= it["Flags"].as<uint16_t>();
				}
			}
			struct DockNodeData
			{
				IGDockNode* Node;
				int64_t Parent = -1;
				int64_t FirstChild = -1;
				int64_t SecondChild = -1;
			};
			std::unordered_map<uint32_t, DockNodeData> nodeMap;
			auto& dockSpace = data["Dockspace"];
			for (auto& it : dockSpace)
			{
				uint32_t id = it["ID"].as<uint32_t>();
				IGDockNode* node = nullptr;
				if (id == 0)
					node = context.Dockspace.m_Root;
				else
					node = context.Dockspace.m_Pool.Allocate<IGDockNode>();
			
				node->Data.Position = it["Position"].as<glm::vec2>();
				node->Data.Size		= it["Size"].as<glm::vec2>();
				node->Type			= (IGSplitType)it["Type"].as<uint32_t>();
				auto& windows		= it["Windows"];
				for (auto& win : windows)
				{
					int32_t hierarchyIndex  = win["HierarchyIndex"].as<int32_t>();
					uint32_t poolIndex		= win["PoolIndex"].as<uint32_t>();
					IGPool& pool			= context.Allocator.GetPools()[poolIndex];
					IGWindow* window		= static_cast<IGWindow*>(pool.GetHierarchy().GetData(hierarchyIndex));
					window->Flags		    |= IGWindow::Docked;
					node->Data.Windows.push_back(window);
					window->Node = node;
				}
				auto parent = it["Parent"];
				auto firstChild = it["FirstChild"];
				auto secondChild = it["SecondChild"];
				DockNodeData nodeData;
				nodeData.Node = node;
				if (parent)
					nodeData.Parent = parent.as<int64_t>();
				if (firstChild)
					nodeData.FirstChild = firstChild.as<int64_t>();
				if (secondChild)
					nodeData.SecondChild = secondChild.as<int64_t>();
				nodeMap[id] = nodeData;
			}
			uint32_t maxID = 0;
			for (auto [id, nodeData] : nodeMap)
			{
				if (id > maxID)
					maxID = id;
				IGDockNode* node = nodeData.Node;
				if (nodeData.Parent != -1)
				{
					IGDockNode* parent = nodeMap[(uint32_t)nodeData.Parent].Node;
					node->Parent = parent;
				}
				if (nodeData.FirstChild != -1)
				{
					IGDockNode* firstChild = nodeMap[(uint32_t)nodeData.FirstChild].Node;
					node->FirstChild = firstChild;
				}
				if (nodeData.SecondChild != -1)
				{
					IGDockNode* secondChild = nodeMap[(uint32_t)nodeData.SecondChild].Node;
					node->SecondChild = secondChild;
				}
			}
			IGDockspace::setNextID(maxID + 1);
			context.Dockspace.UpdateWindows();
		}
		stream.close();
	}
}
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
	void IGSerializer::Serialize(const std::string& filepath, const IGContext& context)
	{
		YAML::Emitter out;
		std::ofstream fout(filepath);

		out << YAML::BeginMap;
		out << YAML::Key << "IG";
		out << YAML::Value << "IG";

		out << YAML::Key << "Root Elements";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& pool : context.Allocator.GetPools())
		{
			for (auto root : pool.GetRootElementIDs())
			{
				const IGWindow* window = static_cast<const IGWindow*>(pool.GetHierarchy().GetData(root));
				out << YAML::BeginMap;
				out << YAML::Key << "Position" << YAML::Value;
				ToVec2(out, window->Position);
				out << YAML::Key << "Size" << YAML::Value;
				ToVec2(out, window->Size);
				out << YAML::Key << "Color" << YAML::Value;
				ToVec4(out, window->Color);
				out << YAML::Key << "FrameColor" << YAML::Value;
				ToVec4(out, window->FrameColor);
				out << YAML::Key << "Flags" << YAML::Value << (uint32_t)window->Flags;
				out << YAML::EndMap; // Window
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		fout << out.c_str();
	}
	void IGSerializer::Deserialize(const std::string& filepath, IGContext& context)
	{
	}
}
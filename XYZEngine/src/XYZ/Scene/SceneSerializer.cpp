#include "stdafx.h"
#include "SceneSerializer.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Gui/GuiContext.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


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

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}

namespace XYZ {
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	static void SerializeInputField(const ECSManager& ecs, YAML::Emitter& out, const InputField& val)
	{
		out << YAML::Key << "InputField";
		out << YAML::BeginMap;

		out << YAML::Key << "SelectColor" << YAML::Value << val.SelectColor;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;
		out << YAML::Key << "TextEntity" << YAML::Value << ecs.GetStorageComponent<IDComponent>(val.TextEntity).ID;

		out << YAML::EndMap;
	}
	static void SerializeRelationship(const ECSManager& ecs, YAML::Emitter& out, const Relationship& val)
	{
		out << YAML::Key << "Relationship";
		out << YAML::BeginMap;

		if (val.Parent != NULL_ENTITY)
			out << YAML::Key << "Parent" << YAML::Value << ecs.GetComponent<IDComponent>(val.Parent).ID;
		if (val.NextSibling != NULL_ENTITY)
			out << YAML::Key << "NextSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.NextSibling).ID;
		if (val.PreviousSibling != NULL_ENTITY)
			out << YAML::Key << "PreviousSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.PreviousSibling).ID;
		if (val.FirstChild != NULL_ENTITY)
			out << YAML::Key << "FirstChild" << YAML::Value << ecs.GetComponent<IDComponent>(val.FirstChild).ID;
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<SceneTagComponent>(YAML::Emitter& out, const SceneTagComponent& val)
	{
		out << YAML::Key << "SceneTagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << val.Name;
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<TransformComponent>(YAML::Emitter& out, const TransformComponent& val)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Position" << YAML::Value << val.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << val.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << val.Scale;

		out << YAML::EndMap; // TransformComponent
	}

	template <>
	void SceneSerializer::serialize<CameraComponent>(YAML::Emitter& out, const CameraComponent& val)
	{
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap; // CameraComponent

		auto& camera = val.Camera;
		auto& perspectiveProps = camera.GetPerspectiveProperties();
		auto& orthoProps = camera.GetOrthographicProperties();
		out << YAML::Key << "ProjectionType" << YAML::Value << ToUnderlying(camera.GetProjectionType());

		out << YAML::Key << "PerspectiveFOV" << YAML::Value << perspectiveProps.PerspectiveFOV;
		out << YAML::Key << "PerspectiveNear" << YAML::Value << perspectiveProps.PerspectiveNear;
		out << YAML::Key << "PerspectiveFar" << YAML::Value << perspectiveProps.PerspectiveFar;

		out << YAML::Key << "OrthographicSize" << YAML::Value << orthoProps.OrthographicSize;
		out << YAML::Key << "OrthographicNear" << YAML::Value << orthoProps.OrthographicNear;
		out << YAML::Key << "OrthographicFar" << YAML::Value << orthoProps.OrthographicFar;

		out << YAML::EndMap; // CameraComponent
	}

	template <>
	void SceneSerializer::serialize<SpriteRenderer>(YAML::Emitter& out, const SpriteRenderer& val)
	{
		out << YAML::Key << "SpriteRenderer";
		out << YAML::BeginMap; // SpriteRenderer

		out << YAML::Key << "MaterialAsset" << YAML::Value << val.Material->Handle;
		out << YAML::Key << "SubTextureAsset" << YAML::Value << val.SubTexture->Handle;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "SortLayer" << YAML::Value << val.SortLayer;
		out << YAML::Key << "IsVisible" << YAML::Value << val.IsVisible;
		out << YAML::EndMap; // SpriteRenderer
	}
	template <>
	void SceneSerializer::serialize<PointLight2D>(YAML::Emitter& out, const PointLight2D& val)
	{
		out << YAML::Key << "PointLight2D";
		out << YAML::BeginMap; // Point Light

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Intensity" << YAML::Value << val.Intensity;
		out << YAML::EndMap; // Point Light
	}
	template <>
	void SceneSerializer::serialize<RectTransform>(YAML::Emitter& out, const RectTransform& val)
	{
		out << YAML::Key << "RectTransformComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "WorldPosition" << YAML::Value << val.WorldPosition;
		out << YAML::Key << "Position" << YAML::Value << val.Position;
		out << YAML::Key << "Size" << YAML::Value << val.Size;
		out << YAML::Key << "Scale" << YAML::Value << val.Scale;

		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<CanvasRenderer>(YAML::Emitter& out, const CanvasRenderer& val)
	{
		out << YAML::Key << "CanvasRenderer";
		out << YAML::BeginMap;

		out << YAML::Key << "MaterialAsset" << YAML::Value << val.Material->Handle;
		out << YAML::Key << "SubTextureAsset" << YAML::Value << val.SubTexture->Handle;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "SortLayer" << YAML::Value << val.SortLayer;
		out << YAML::Key << "IsVisible" << YAML::Value << val.IsVisible;
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<LineRenderer>(YAML::Emitter& out, const LineRenderer& val)
	{
		out << YAML::Key << "LineRenderer";
		out << YAML::BeginMap;

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "IsVisible" << YAML::Value << val.IsVisible;

		out << YAML::Key << "Points";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& it : val.LineMesh.Points)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Point" << YAML::Value << it;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // LineRenderer
	}

	template <>
	void SceneSerializer::serialize<LayoutGroup>(YAML::Emitter& out, const LayoutGroup& val)
	{
		out << YAML::Key << "LayoutGroup";
		out << YAML::BeginMap;

		out << YAML::Key << "LeftPadding" << YAML::Value << val.Padding.Left;
		out << YAML::Key << "RightPadding" << YAML::Value << val.Padding.Right;
		out << YAML::Key << "TopPadding" << YAML::Value << val.Padding.Top;
		out << YAML::Key << "BottomPadding" << YAML::Value << val.Padding.Bottom;
		out << YAML::Key << "CellSpacing" << YAML::Value << val.CellSpacing;

		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<Button>(YAML::Emitter& out, const Button& val)
	{
		out << YAML::Key << "Button";
		out << YAML::BeginMap;

		out << YAML::Key << "ClickColor" << YAML::Value << val.ClickColor;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<Checkbox>(YAML::Emitter& out, const Checkbox& val)
	{
		out << YAML::Key << "Checkbox";
		out << YAML::BeginMap;

		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
	}
	template <>
	void SceneSerializer::serialize<Slider>(YAML::Emitter& out, const Slider& val)
	{
		out << YAML::Key << "Slider";
		out << YAML::BeginMap;

		out << YAML::Key << "Value" << YAML::Value << val.Value;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
	}


	template <>
	void SceneSerializer::serialize<Canvas>(YAML::Emitter& out, const Canvas& val)
	{
		out << YAML::Key << "Canvas";
		out << YAML::BeginMap;

		out << YAML::Key << "Color" << YAML::Value << val.Color;

		out << YAML::EndMap;
	}


	template <>
	void SceneSerializer::serialize<Text>(YAML::Emitter& out, const Text& val)
	{
		out << YAML::Key << "Text";
		out << YAML::BeginMap;

		out << YAML::Key << "FontAsset" << YAML::Value << val.Font->Handle;
		out << YAML::Key << "Source" << YAML::Value << val.Source;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Alignment" << YAML::Value << ToUnderlying(val.Alignment);


		out << YAML::EndMap; // Text
	}

	template <>
	void SceneSerializer::serialize<RigidBody2DComponent>(YAML::Emitter& out, const RigidBody2DComponent& val)
	{
		out << YAML::Key << "RigidBody2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Type" << YAML::Value << ToUnderlying(val.Type);
		out << YAML::EndMap; // RigidBody2D
	}

	template <>
	void SceneSerializer::serialize<BoxCollider2DComponent>(YAML::Emitter& out, const BoxCollider2DComponent& val)
	{
		out << YAML::Key << "BoxCollider2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Offset" << val.Offset;
		out << YAML::Key << "Size" << val.Size;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::EndMap; // BoxCollider2D;
	}

	template <>
	void SceneSerializer::serialize<CircleCollider2DComponent>(YAML::Emitter& out, const CircleCollider2DComponent& val)
	{
		out << YAML::Key << "CircleCollider2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Offset" << val.Offset;
		out << YAML::Key << "Radius" << val.Radius;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::EndMap; // CircleCollider2D;
	}


	template<>
	void SceneSerializer::serialize<SceneEntity>(YAML::Emitter& out, const SceneEntity& val)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << val.GetComponent<IDComponent>().ID;

		if (val.HasComponent<SceneTagComponent>())
		{
			serialize<SceneTagComponent>(out, val.GetComponent<SceneTagComponent>());
		}
		if (val.HasComponent<TransformComponent>())
		{
			serialize<TransformComponent>(out, val.GetComponent<TransformComponent>());
		}
		if (val.HasComponent<CameraComponent>())
		{
			serialize<CameraComponent>(out, val.GetComponent<CameraComponent>());
		}
		if (val.HasComponent<SpriteRenderer>())
		{
			serialize<SpriteRenderer>(out, val.GetComponent<SpriteRenderer>());
		}
		if (val.HasComponent<Button>())
		{
			serialize<Button>(out, val.GetComponent<Button>());
		}
		if (val.HasComponent<Checkbox>())
		{
			serialize<Checkbox>(out, val.GetComponent<Checkbox>());
		}
		if (val.HasComponent<Slider>())
		{
			serialize<Slider>(out, val.GetComponent<Slider>());
		}
		if (val.HasComponent<InputField>())
		{
			SerializeInputField(val.m_Scene->m_ECS, out, val.GetComponent<InputField>());
		}
		if (val.HasComponent<LayoutGroup>())
		{
			serialize<LayoutGroup>(out, val.GetComponent<LayoutGroup>());
		}
		if (val.HasComponent<Relationship>())
		{
			SerializeRelationship(val.m_Scene->m_ECS, out, val.GetComponent<Relationship>());
		}
		if (val.HasComponent<CanvasRenderer>())
		{
			serialize<CanvasRenderer>(out, val.GetComponent<CanvasRenderer>());
		}
		if (val.HasComponent<RectTransform>())
		{
			serialize<RectTransform>(out, val.GetComponent<RectTransform>());
		}
		if (val.HasComponent<Canvas>())
		{
			serialize<Canvas>(out, val.GetComponent<Canvas>());
		}
		if (val.HasComponent<Text>())
		{
			serialize<Text>(out, val.GetComponent<Text>());
		}

		if (val.HasComponent<LineRenderer>())
		{
			serialize<LineRenderer>(out, val.GetComponent<LineRenderer>());
		}

		if (val.HasComponent<RigidBody2DComponent>())
		{
			serialize<RigidBody2DComponent>(out, val.GetComponent<RigidBody2DComponent>());
		}
		if (val.HasComponent<BoxCollider2DComponent>())
		{
			serialize<BoxCollider2DComponent>(out, val.GetComponent<BoxCollider2DComponent>());
		}
		if (val.HasComponent<CircleCollider2DComponent>())
		{
			serialize<CircleCollider2DComponent>(out, val.GetComponent<CircleCollider2DComponent>());
		}
		out << YAML::EndMap; // Entity
	}


	SceneSerializer::SceneSerializer(Ref<Scene> scene)
		:
		m_Scene(scene)
	{
	}
	void SceneSerializer::Serialize()
	{
		YAML::Emitter out;
		
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << m_Scene->m_Name;
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (auto ent : m_Scene->m_Entities)
		{
			SceneEntity entity(ent, m_Scene.Raw());
			serialize<SceneEntity>(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(m_Scene->FilePath);
		fout << out.c_str();
	}
	Ref<Scene> SceneSerializer::Deserialize()
	{
	}
}
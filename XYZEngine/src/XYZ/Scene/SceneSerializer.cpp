#include "stdafx.h"
#include "SceneSerializer.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Asset/AssetManager.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {

	static void SerializeRelationship(ECSManager& ecs, YAML::Emitter& out, const Relationship& val)
	{
		out << YAML::Key << "Relationship";
		out << YAML::BeginMap;

		if ((bool)val.Parent)
			out << YAML::Key << "Parent" << YAML::Value << ecs.GetComponent<IDComponent>(val.Parent).ID;
		if ((bool)val.NextSibling)
			out << YAML::Key << "NextSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.NextSibling).ID;
		if ((bool)val.PreviousSibling)
			out << YAML::Key << "PreviousSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.PreviousSibling).ID;
		if ((bool)val.FirstChild)
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
		if (val.HasComponent<Relationship>())
		{
			SerializeRelationship(val.m_Scene->m_ECS, out, val.GetComponent<Relationship>());
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


	template <>
	SpriteRenderer SceneSerializer::deserialize<SpriteRenderer>(YAML::Node& data)
	{
		GUID materialHandle(data["MaterialAsset"].as<std::string>());
		GUID subTextureHandle(data["SubTextureAsset"].as<std::string>());
		glm::vec4 color = data["Color"].as<glm::vec4>();
		uint16_t sortLayer = data["SortLayer"].as<uint16_t>();

		Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
		Ref<SubTexture> subTexture = AssetManager::GetAsset<SubTexture>(subTextureHandle);

		SpriteRenderer spriteRenderer(
			material,
			subTexture,
			color,
			sortLayer
		);
		return spriteRenderer;
	}

	template <>
	CameraComponent SceneSerializer::deserialize<CameraComponent>(YAML::Node& data)
	{
		CameraPerspectiveProperties perspectiveProps;
		CameraOrthographicProperties orthoProps;
		CameraProjectionType projectionType;

		uint32_t type = data["ProjectionType"].as<uint32_t>();
		switch (type)
		{
		case ToUnderlying(CameraProjectionType::Orthographic):
			projectionType = CameraProjectionType::Orthographic;
			break;
		case ToUnderlying(CameraProjectionType::Perspective):
			projectionType = CameraProjectionType::Perspective;
			break;
		default:
			projectionType = CameraProjectionType::Orthographic;
		}
		perspectiveProps.PerspectiveFOV = data["PerspectiveFOV"].as<float>();
		perspectiveProps.PerspectiveNear = data["PerspectiveNear"].as<float>();
		perspectiveProps.PerspectiveFar = data["PerspectiveFar"].as<float>();

		orthoProps.OrthographicSize = data["OrthographicSize"].as<float>();
		orthoProps.OrthographicNear = data["OrthographicNear"].as<float>();
		orthoProps.OrthographicFar = data["OrthographicFar"].as<float>();

		CameraComponent camera;
		camera.Camera.SetProjectionType(projectionType);
		camera.Camera.SetPerspective(perspectiveProps);
		camera.Camera.SetOrthographic(orthoProps);
		return camera;
	}

	template <>
	TransformComponent SceneSerializer::deserialize<TransformComponent>(YAML::Node& data)
	{
		TransformComponent transform(data["Position"].as<glm::vec3>());

		glm::vec3 rotation = data["Rotation"].as<glm::vec3>();
		glm::vec3 scale = data["Scale"].as<glm::vec3>();

		transform.Rotation = rotation;
		transform.Scale = scale;
		return transform;
	}

	template <>
	PointLight2D SceneSerializer::deserialize<PointLight2D>(YAML::Node& data)
	{
		PointLight2D light;
		light.Color = data["Color"].as<glm::vec3>();
		light.Intensity = data["Intensity"].as<float>();
		return light;
	}

	static Relationship DeserializeRelationship(const ECSManager& ecs, YAML::Node& data)
	{
		Relationship relationship;
		if (data["Parent"])
		{
			std::string parent = data["Parent"].as<std::string>();
			relationship.Parent = ecs.FindEntity<IDComponent>(IDComponent({ parent }));
		}
		if (data["NextSibling"])
		{
			std::string nextSibling = data["NextSibling"].as<std::string>();
			relationship.NextSibling = ecs.FindEntity<IDComponent>(IDComponent({ nextSibling }));
		}
		if (data["PreviousSibling"])
		{
			std::string previousSibling = data["PreviousSibling"].as<std::string>();
			relationship.PreviousSibling = ecs.FindEntity<IDComponent>(IDComponent({ previousSibling }));
		}
		if (data["FirstChild"])
		{
			std::string firstChild = data["FirstChild"].as<std::string>();
			relationship.FirstChild = ecs.FindEntity<IDComponent>(IDComponent({ firstChild }));
		}
		return relationship;
	}

	template <>
	RigidBody2DComponent SceneSerializer::deserialize<RigidBody2DComponent>(YAML::Node& data)
	{
		RigidBody2DComponent body;
		uint32_t type = data["Type"].as<uint32_t>();
		switch (type)
		{
		case ToUnderlying(RigidBody2DComponent::BodyType::Static):
			body.Type = RigidBody2DComponent::BodyType::Static;
			break;
		case ToUnderlying(RigidBody2DComponent::BodyType::Dynamic):
			body.Type = RigidBody2DComponent::BodyType::Dynamic;
			break;
		case ToUnderlying(RigidBody2DComponent::BodyType::Kinematic):
			body.Type = RigidBody2DComponent::BodyType::Kinematic;
			break;
		}
		return body;
	}

	template <>
	BoxCollider2DComponent SceneSerializer::deserialize<BoxCollider2DComponent>(YAML::Node& data)
	{
		BoxCollider2DComponent box;
		box.Offset = data["Offset"].as<glm::vec2>();
		box.Size = data["Size"].as<glm::vec2>();
		box.Density = data["Density"].as<float>();

		return box;
	}
	template <>
	CircleCollider2DComponent SceneSerializer::deserialize<CircleCollider2DComponent>(YAML::Node& data)
	{
		CircleCollider2DComponent circle;
		circle.Offset = data["Offset"].as<glm::vec2>();
		circle.Radius = data["Radius"].as<float>();
		circle.Density = data["Density"].as<float>();

		return circle;
	}


	template <>
	SceneEntity SceneSerializer::deserialize<SceneEntity>(YAML::Node& data)
	{
		GUID guid;
		guid = data["Entity"].as<std::string>();
		auto& tagComponent = data["SceneTagComponent"];
		
		SceneTagComponent tag = tagComponent["Name"].as<std::string>();
		SceneEntity entity = m_Scene->CreateEntity(tag, guid);



		auto transformComponent = data["TransformComponent"];
		if (transformComponent)
		{
			entity.GetComponent<TransformComponent>() = SceneSerializer::deserialize<TransformComponent>(transformComponent);
		}

		auto cameraComponent = data["CameraComponent"];
		if (cameraComponent)
		{
			entity.AddComponent<CameraComponent>(deserialize<CameraComponent>(cameraComponent));
		}
		auto spriteRenderer = data["SpriteRenderer"];
		if (spriteRenderer)
		{
			entity.AddComponent<SpriteRenderer>(deserialize<SpriteRenderer>(spriteRenderer));
		}
		auto pointLight = data["PointLight2D"];
		if (pointLight)
		{
			entity.AddComponent<PointLight2D>(deserialize<PointLight2D>(pointLight));
		}

		auto rigidBody = data["RigidBody2D"];
		if (rigidBody)
		{
			entity.AddComponent<RigidBody2DComponent>(deserialize<RigidBody2DComponent>(rigidBody));
		}

		auto boxCollider = data["BoxCollider2D"];
		if (boxCollider)
		{
			entity.AddComponent<BoxCollider2DComponent>(deserialize<BoxCollider2DComponent>(boxCollider));
		}

		auto circleCollider = data["CircleCollider2D"];
		if (circleCollider)
		{
			entity.AddComponent<CircleCollider2DComponent>(deserialize<CircleCollider2DComponent>(circleCollider));
		}
		return entity;
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
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
		std::ifstream stream(m_Scene->FilePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		m_Scene->m_Name = data["Scene"].as<std::string>();
		m_Scene->m_ECS.GetComponent<SceneTagComponent>(m_Scene->m_SceneEntity).Name = m_Scene->m_Name;
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				SceneSerializer::deserialize<SceneEntity>(entity);
			}
		}
		return m_Scene;
	}
}
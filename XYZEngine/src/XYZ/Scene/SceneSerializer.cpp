#include "stdafx.h"
#include "SceneSerializer.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Script/ScriptEngine.h"

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

		if ((bool)val.GetParent())
			out << YAML::Key << "Parent" << YAML::Value << ecs.GetComponent<IDComponent>(val.GetParent()).ID;
		if ((bool)val.GetNextSibling())
			out << YAML::Key << "NextSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.GetNextSibling()).ID;
		if ((bool)val.GetPreviousSibling())
			out << YAML::Key << "PreviousSibling" << YAML::Value << ecs.GetComponent<IDComponent>(val.GetPreviousSibling()).ID;
		if ((bool)val.GetFirstChild())
			out << YAML::Key << "FirstChild" << YAML::Value << ecs.GetComponent<IDComponent>(val.GetFirstChild()).ID;
		out << YAML::Key << "Depth" << YAML::Value << val.GetDepth();
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

		//out << YAML::Key << "MaterialAsset" << YAML::Value << val.Material->Handle;
		//out << YAML::Key << "SubTextureAsset" << YAML::Value << val.SubTexture->Handle;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "SortLayer" << YAML::Value << val.SortLayer;
		out << YAML::Key << "Visible" << YAML::Value << val.Visible;
		out << YAML::EndMap; // SpriteRenderer
	}
	template <>
	void SceneSerializer::serialize<PointLight2D>(YAML::Emitter& out, const PointLight2D& val)
	{
		out << YAML::Key << "PointLight2D";
		out << YAML::BeginMap; // Point Light

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Radius" << YAML::Value << val.Radius;
		out << YAML::Key << "Intensity" << YAML::Value << val.Intensity;
		out << YAML::EndMap; // Point Light
	}
	template <>
	void SceneSerializer::serialize<SpotLight2D>(YAML::Emitter& out, const SpotLight2D& val)
	{
		out << YAML::Key << "SpotLight2D";
		out << YAML::BeginMap; // Spot Light

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Radius" << YAML::Value << val.Radius;
		out << YAML::Key << "Intensity" << YAML::Value << val.Intensity;
		out << YAML::Key << "OuterAngle" << YAML::Value << val.OuterAngle;
		out << YAML::Key << "InnerAngle" << YAML::Value << val.InnerAngle;
		out << YAML::EndMap; // Spot Light
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
		out << YAML::Key << "Size" << val.Size;
		out << YAML::Key << "Offset" << val.Offset;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::Key << "Friction" << val.Friction;
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
		out << YAML::Key << "Friction" << val.Friction;
		out << YAML::EndMap; // CircleCollider2D;
	}
	template <>
	void SceneSerializer::serialize<ChainCollider2DComponent>(YAML::Emitter& out, const ChainCollider2DComponent& val)
	{
		out << YAML::Key << "ChainCollider2D";
		out << YAML::BeginMap;
		

		out << YAML::Key << "Points" << YAML::Value << YAML::BeginSeq;
		for (auto& p : val.Points)
		{
			out << YAML::Value << p;
		}
		out << YAML::EndSeq;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::Key << "Friction" << val.Friction;
		out << YAML::EndMap; // ChainCollider2D;
	}

	template <>
	void SceneSerializer::serialize<ScriptComponent>(YAML::Emitter& out, const ScriptComponent& val)
	{
		out << YAML::Key << "ScriptComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "ModuleName" << val.ModuleName;
		for (auto& field : val.GetFields())
		{
			out << YAML::Key << field.GetName();
			if (field.GetType() == PublicFieldType::Float)
			{
				out << YAML::Value << field.GetStoredValue<float>();
			}
			else if (field.GetType() == PublicFieldType::Int)
			{
				out << YAML::Value << field.GetStoredValue<int32_t>();
			}
			else if (field.GetType() == PublicFieldType::UnsignedInt)
			{
				out << YAML::Value << field.GetStoredValue<uint32_t>();
			}
			else if (field.GetType() == PublicFieldType::String)
			{
				out << YAML::Value << std::string(field.GetStoredValue<char*>());
			}
			else if (field.GetType() == PublicFieldType::Vec2)
			{
				out << YAML::Value << field.GetStoredValue<glm::vec2>();
			}
			else if (field.GetType() == PublicFieldType::Vec3)
			{
				out << YAML::Value << field.GetStoredValue<glm::vec3>();
			}
			else if (field.GetType() == PublicFieldType::Vec4)
			{
				out << YAML::Value << field.GetStoredValue<glm::vec4>();
			}
		}
		out << YAML::EndMap; // Script Component
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
		if (val.HasComponent<ChainCollider2DComponent>())
		{
			serialize<ChainCollider2DComponent>(out, val.GetComponent<ChainCollider2DComponent>());
		}
		if (val.HasComponent<ScriptComponent>())
		{
			serialize<ScriptComponent>(out, val.GetComponent<ScriptComponent>());
		}
		if (val.HasComponent<PointLight2D>())
		{
			serialize<PointLight2D>(out, val.GetComponent<PointLight2D>());
		}
		if (val.HasComponent<SpotLight2D>())
		{
			serialize<SpotLight2D>(out, val.GetComponent<SpotLight2D>());
		}
		out << YAML::EndMap; // Entity
	}


	template <>
	void SceneSerializer::deserialize<ScriptComponent>(YAML::Node& data, SceneEntity entity)
	{
		ScriptComponent scriptComponent;
		scriptComponent.ModuleName = data["ModuleName"].as<std::string>();
		entity.AddComponent(scriptComponent);

		const auto& component = entity.GetComponent<ScriptComponent>();
		for (auto& field : component.GetFields())
		{
			auto val = data[field.GetName()];
			if (!val)
				continue;
			if (field.GetType() == PublicFieldType::Float)
			{
				field.SetStoredValue<float>(val.as<float>());
			}
			else if (field.GetType() == PublicFieldType::Int)
			{
				field.SetStoredValue<int32_t>(val.as<int32_t>());
			}
			else if (field.GetType() == PublicFieldType::UnsignedInt)
			{
				field.SetStoredValue<uint32_t>(val.as<uint32_t>());
			}
			else if (field.GetType() == PublicFieldType::String)
			{
				std::string str = val.as<std::string>();
				field.SetStoredValue<const char*>(str.c_str());
			}
			else if (field.GetType() == PublicFieldType::Vec2)
			{
				field.SetStoredValue <glm::vec2>(val.as<glm::vec2>());
			}
			else if (field.GetType() == PublicFieldType::Vec3)
			{
				field.SetStoredValue <glm::vec3>(val.as<glm::vec3>());
			}
			else if (field.GetType() == PublicFieldType::Vec4)
			{
				field.SetStoredValue <glm::vec4>(val.as<glm::vec4>());
			}
		}
	}

	template <>
	void SceneSerializer::deserialize<SpriteRenderer>(YAML::Node& data, SceneEntity entity)
	{
		const GUID materialHandle(data["MaterialAsset"].as<std::string>());
		const GUID subTextureHandle(data["SubTextureAsset"].as<std::string>());
		const glm::vec4 color		= data["Color"].as<glm::vec4>();
		const uint16_t sortLayer  = data["SortLayer"].as<uint16_t>();
		const bool visible	    = data["Visible"].as<bool>();

		const Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
		const Ref<SubTexture> subTexture = AssetManager::GetAsset<SubTexture>(subTextureHandle);

		const SpriteRenderer spriteRenderer(
			material,
			subTexture,
			color,
			sortLayer,
			visible
		);
		entity.AddComponent(spriteRenderer);
	}

	template <>
	void SceneSerializer::deserialize<CameraComponent>(YAML::Node& data, SceneEntity entity)
	{
		CameraPerspectiveProperties perspectiveProps;
		CameraOrthographicProperties orthoProps;
		CameraProjectionType projectionType;

		const uint32_t type = data["ProjectionType"].as<uint32_t>();
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
		entity.AddComponent(camera);
	}

	template <>
	void SceneSerializer::deserialize<TransformComponent>(YAML::Node& data, SceneEntity entity)
	{
		TransformComponent transform(data["Position"].as<glm::vec3>());

		const glm::vec3 rotation = data["Rotation"].as<glm::vec3>();
		const glm::vec3 scale = data["Scale"].as<glm::vec3>();

		transform.Rotation = rotation;
		transform.Scale = scale;
		entity.GetComponent<TransformComponent>() = transform;
	}

	template <>
	void SceneSerializer::deserialize<PointLight2D>(YAML::Node& data, SceneEntity entity)
	{
		PointLight2D light;
		light.Color = data["Color"].as<glm::vec3>();
		light.Radius = data["Radius"].as<float>();
		light.Intensity = data["Intensity"].as<float>();
		entity.AddComponent(light);
	}
	template <>
	void SceneSerializer::deserialize<SpotLight2D>(YAML::Node& data, SceneEntity entity)
	{
		SpotLight2D light;
		light.Color = data["Color"].as<glm::vec3>();
		light.Radius = data["Radius"].as<float>();
		light.Intensity = data["Intensity"].as<float>();
		light.OuterAngle = data["OuterAngle"].as<float>();
		light.InnerAngle = data["InnerAngle"].as<float>();
		entity.AddComponent(light);
	}

	template <>
	void SceneSerializer::deserialize<RigidBody2DComponent>(YAML::Node& data, SceneEntity entity)
	{
		RigidBody2DComponent body;
		const uint32_t type = data["Type"].as<uint32_t>();
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
		entity.AddComponent(body);
	}

	template <>
	void SceneSerializer::deserialize<BoxCollider2DComponent>(YAML::Node& data, SceneEntity entity)
	{
		BoxCollider2DComponent box;

		box.Offset = data["Offset"].as<glm::vec2>();
		box.Size = data["Size"].as<glm::vec2>();
		box.Density = data["Density"].as<float>();
		box.Friction = data["Friction"].as<float>();

		entity.AddComponent<BoxCollider2DComponent>(box);
	}
	template <>
	void SceneSerializer::deserialize<CircleCollider2DComponent>(YAML::Node& data, SceneEntity entity)
	{
		CircleCollider2DComponent circle;

		circle.Offset = data["Offset"].as<glm::vec2>();
		circle.Radius = data["Radius"].as<float>();
		circle.Density = data["Density"].as<float>();
		circle.Friction = data["Friction"].as<float>();

		entity.AddComponent(circle);
	}
	template <>
	void SceneSerializer::deserialize<ChainCollider2DComponent>(YAML::Node& data, SceneEntity entity)
	{
		ChainCollider2DComponent chain;

		chain.Points = data["Points"].as<std::vector<glm::vec2>>();
		chain.Density = data["Density"].as<float>();
		chain.Friction = data["Friction"].as<float>();

		entity.AddComponent(chain);
	}

	template <>
	void SceneSerializer::deserialize<SceneEntity>(YAML::Node& data, SceneEntity ent)
	{
		GUID guid = data["Entity"].as<std::string>();
		auto& tagComponent = data["SceneTagComponent"];
		
		SceneTagComponent tag = tagComponent["Name"].as<std::string>();
		SceneEntity entity = m_Scene->CreateEntity(tag, guid);

		auto transformComponent = data["TransformComponent"];
		if (transformComponent)
		{
			SceneSerializer::deserialize<TransformComponent>(transformComponent, entity);
		}

		auto cameraComponent = data["CameraComponent"];
		if (cameraComponent)
		{
			deserialize<CameraComponent>(cameraComponent, entity);
		}
		auto spriteRenderer = data["SpriteRenderer"];
		if (spriteRenderer)
		{
			deserialize<SpriteRenderer>(spriteRenderer, entity);
		}

		auto rigidBody = data["RigidBody2D"];
		if (rigidBody)
		{
			deserialize<RigidBody2DComponent>(rigidBody, entity);
		}

		auto boxCollider = data["BoxCollider2D"];
		if (boxCollider)
		{
			deserialize<BoxCollider2DComponent>(boxCollider, entity);
		}

		auto circleCollider = data["CircleCollider2D"];
		if (circleCollider)
		{
			deserialize<CircleCollider2DComponent>(circleCollider, entity);
		}

		auto chainCollider = data["ChainCollider2D"];
		if (chainCollider)
		{
			deserialize<ChainCollider2DComponent>(chainCollider, entity);
		}

		auto scriptComponent = data["ScriptComponent"];
		if (scriptComponent)
		{
			deserialize<ScriptComponent>(scriptComponent, entity);
		}
			
		auto pointLightComponent = data["PointLight2D"];
		if (pointLightComponent)
		{
			deserialize<PointLight2D>(pointLightComponent, entity);
		}

		auto spotLightComponent = data["SpotLight2D"];
		if (spotLightComponent)
		{
			deserialize<SpotLight2D>(spotLightComponent, entity);
		}
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:
		m_Scene(scene)
	{
	}
	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << m_Scene->m_Name;
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (const auto ent : m_Scene->m_Entities)
		{
			SceneEntity entity(ent, m_Scene.Raw());
			auto& rel = entity.GetComponent<Relationship>();
			if (rel.Parent == m_Scene->GetSceneEntity())
				rel.Parent = Entity();
			serialize<SceneEntity>(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}



	Ref<Scene> SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		m_Scene->m_Name = data["Scene"].as<std::string>();
		ECSManager& ecs = m_Scene->m_ECS;
		ecs.GetComponent<SceneTagComponent>(m_Scene->m_SceneEntity).Name = m_Scene->m_Name;
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				SceneSerializer::deserialize<SceneEntity>(entity, SceneEntity());
			}
			for (auto data : entities)
			{
				GUID guid = data["Entity"].as<std::string>();
				Entity entity = ecs.FindEntity<IDComponent>(IDComponent(guid));
				Relationship& relationship = ecs.GetComponent<Relationship>(entity);
				auto relComponent = data["Relationship"];
				if (relComponent["Parent"])
				{
					std::string parent = relComponent["Parent"].as<std::string>();
					Entity parentEntity = ecs.FindEntity<IDComponent>(IDComponent({ parent }));
					if (ecs.IsValid(parentEntity))
					{
						// Remove relation with scene entity
						Relationship::RemoveRelation(entity, ecs);
						relationship.Parent = parentEntity;					
					}
				}
				if (relComponent["NextSibling"])
				{
					std::string nextSibling = relComponent["NextSibling"].as<std::string>();
					relationship.NextSibling = ecs.FindEntity<IDComponent>(IDComponent({ nextSibling }));
				}
				if (relComponent["PreviousSibling"])
				{
					std::string previousSibling = relComponent["PreviousSibling"].as<std::string>();
					relationship.PreviousSibling = ecs.FindEntity<IDComponent>(IDComponent({ previousSibling }));
				}
				if (relComponent["FirstChild"])
				{
					std::string firstChild = relComponent["FirstChild"].as<std::string>();
					relationship.FirstChild = ecs.FindEntity<IDComponent>(IDComponent({ firstChild }));
				}
				relationship.Depth = relComponent["Depth"].as<uint32_t>();
			}
		}
		return m_Scene;
	}
}
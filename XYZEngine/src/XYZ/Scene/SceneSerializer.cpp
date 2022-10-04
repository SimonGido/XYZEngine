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

	static void SerializeRelationship(entt::registry& reg, YAML::Emitter& out, const Relationship& val)
	{
		out << YAML::Key << "Relationship";
		out << YAML::BeginMap;
	

		if (reg.valid(val.GetParent()))
			out << YAML::Key << "Parent" << YAML::Value << reg.get<IDComponent>(val.GetParent()).ID;
		if (reg.valid(val.GetNextSibling()))
			out << YAML::Key << "NextSibling" << YAML::Value << reg.get<IDComponent>(val.GetNextSibling()).ID;
		if (reg.valid(val.GetPreviousSibling()))
			out << YAML::Key << "PreviousSibling" << YAML::Value << reg.get<IDComponent>(val.GetPreviousSibling()).ID;
		if (reg.valid(val.GetFirstChild()))
			out << YAML::Key << "FirstChild" << YAML::Value << reg.get<IDComponent>(val.GetFirstChild()).ID;
		out << YAML::Key << "Depth" << YAML::Value << val.GetDepth();
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<SceneTagComponent>(YAML::Emitter& out, const SceneTagComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "SceneTagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << val.Name;
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<TransformComponent>(YAML::Emitter& out, const TransformComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Position" << YAML::Value << val.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << val.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << val.Scale;

		out << YAML::EndMap; // TransformComponent
	}

	template <>
	void SceneSerializer::serialize<CameraComponent>(YAML::Emitter& out, const CameraComponent& val, SceneEntity entity)
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
	void SceneSerializer::serialize<SpriteRenderer>(YAML::Emitter& out, const SpriteRenderer& val, SceneEntity entity)
	{
		out << YAML::Key << "SpriteRenderer";
		out << YAML::BeginMap; // SpriteRenderer

		AssetHandle materialHandle;
		AssetHandle subTextureHandle;
		
		if (val.Material.Raw())
			materialHandle = val.Material->GetHandle();
		if (val.SubTexture.Raw())
			subTextureHandle = val.SubTexture->GetHandle();
		
		out << YAML::Key << "Material" << YAML::Value << materialHandle;
		out << YAML::Key << "SubTexture" << YAML::Value << subTextureHandle;
		out << YAML::Key << "Color"		<< YAML::Value << val.Color;
		out << YAML::Key << "SortLayer"	<< YAML::Value << val.SortLayer;
		out << YAML::Key << "Visible"	<< YAML::Value << val.Visible;
		out << YAML::EndMap; // SpriteRenderer
	}
	template <>
	void SceneSerializer::serialize<PointLight2D>(YAML::Emitter& out, const PointLight2D& val, SceneEntity entity)
	{
		out << YAML::Key << "PointLight2D";
		out << YAML::BeginMap; // Point Light

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Radius" << YAML::Value << val.Radius;
		out << YAML::Key << "Intensity" << YAML::Value << val.Intensity;
		out << YAML::EndMap; // Point Light
	}
	template <>
	void SceneSerializer::serialize<SpotLight2D>(YAML::Emitter& out, const SpotLight2D& val, SceneEntity entity)
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
	void SceneSerializer::serialize<RigidBody2DComponent>(YAML::Emitter& out, const RigidBody2DComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "RigidBody2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Type" << YAML::Value << ToUnderlying(val.Type);
		out << YAML::EndMap; // RigidBody2D
	}

	template <>
	void SceneSerializer::serialize<BoxCollider2DComponent>(YAML::Emitter& out, const BoxCollider2DComponent& val, SceneEntity entity)
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
	void SceneSerializer::serialize<CircleCollider2DComponent>(YAML::Emitter& out, const CircleCollider2DComponent& val, SceneEntity entity)
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
	void SceneSerializer::serialize<ChainCollider2DComponent>(YAML::Emitter& out, const ChainCollider2DComponent& val, SceneEntity entity)
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
		out << YAML::Key << "InvertNormals" << val.InvertNormals;
		out << YAML::EndMap; // ChainCollider2D;
	}

	template <>
	void SceneSerializer::serialize<ScriptComponent>(YAML::Emitter& out, const ScriptComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "ScriptComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "ModuleName" << val.ModuleName;
		auto& fields = ScriptEngine::GetPublicFields(entity);
		for (auto& field : fields)
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
	template <>
	void SceneSerializer::serialize<ParticleRenderer>(YAML::Emitter& out, const ParticleRenderer& val, SceneEntity entity)
	{
		out << YAML::Key << "ParticleRenderer";
		out << YAML::BeginMap;

		out << YAML::Key << "Material" << val.MaterialAsset->GetHandle();
		out << YAML::Key << "Mesh" << val.Mesh->GetHandle();

		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<ParticleComponent>(YAML::Emitter& out, const ParticleComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "ParticleComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MaxParticles" << val.GetSystem()->MaxParticles;
		out << YAML::Key << "Speed" << val.GetSystem()->Speed;
		
		out << YAML::Key << "AnimationTiles" << val.GetSystem()->AnimationTiles;
		out << YAML::Key << "AnimationStartFrame" << val.GetSystem()->AnimationStartFrame;
		out << YAML::Key << "AnimationCycleLength" << val.GetSystem()->AnimationCycleLength;

		out << YAML::Key << "EndRotation" << val.GetSystem()->EndRotation;
		out << YAML::Key << "EndSize" << val.GetSystem()->EndSize;
		out << YAML::Key << "EndColor" << val.GetSystem()->EndColor;
		
		out << YAML::Key << "LightEndColor" << val.GetSystem()->LightEndColor;
		out << YAML::Key << "LightEndIntensity" << val.GetSystem()->LightEndIntensity;
		out << YAML::Key << "LightEndRadius" << val.GetSystem()->LightEndRadius;
		
		// Modules enabled
		out << YAML::Key << "ModulesEnabled";
		out << YAML::Value << YAML::BeginSeq;
		out << YAML::Flow;
		for (bool enabled : val.ModulesEnabled)
			out << YAML::Value << enabled;
		out << YAML::EndSeq;

		{  // Emitter
			out << YAML::Key << "Emitter";
			out << YAML::BeginMap; // Emitter
		
			out << YAML::Key << "Shape" << static_cast<uint32_t>(val.GetSystem()->Emitter.Shape);
			out << YAML::Key << "BoxMin" <<  val.GetSystem()->Emitter.BoxMin;
			out << YAML::Key << "BoxMax" <<  val.GetSystem()->Emitter.BoxMax;
			out << YAML::Key << "Radius" <<  val.GetSystem()->Emitter.Radius;
		
			out << YAML::Key << "EmitRate" << val.GetSystem()->Emitter.EmitRate;
			out << YAML::Key << "LifeTime" << val.GetSystem()->Emitter.LifeTime;
			
			out << YAML::Key << "MinVelocity" << val.GetSystem()->Emitter.MinVelocity;
			out << YAML::Key << "MaxVelocity" << val.GetSystem()->Emitter.MaxVelocity;
			
			out << YAML::Key << "Size" <<    val.GetSystem()->Emitter.Size;
			out << YAML::Key << "Color" <<   val.GetSystem()->Emitter.Color;
		
			out << YAML::Key << "LightColor" << val.GetSystem()->Emitter.LightColor;
			out << YAML::Key << "LightRadius" << val.GetSystem()->Emitter.LightRadius;
			out << YAML::Key << "LightIntensity" << val.GetSystem()->Emitter.LightIntensity;
			out << YAML::Key << "MaxLights" << val.GetSystem()->Emitter.MaxLights;

			out << YAML::Key << "BurstInterval" << val.GetSystem()->Emitter.BurstInterval;
			{
				out << YAML::Key << "Bursts";
				out << YAML::Value << YAML::BeginSeq;
				for (const auto& burst : val.GetSystem()->Emitter.Bursts)
				{
					out << YAML::BeginMap; // Burst
					out << YAML::Key << "Count" << burst.Count;
					out << YAML::Key << "Time" << burst.Time;
					out << YAML::Key << "Probability" << burst.Probability;
					out << YAML::EndMap; // Burst
				}
				out << YAML::EndSeq;
			}
			out << YAML::EndMap; // Emitter
		}
		out << YAML::EndMap; // Particle Component
	}
	template <>
	void SceneSerializer::serialize<MeshComponent>(YAML::Emitter& out, const MeshComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "MeshComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Mesh" << val.Mesh->GetHandle();
		out << YAML::Key << "Material" << val.MaterialAsset->GetHandle();
		out << YAML::EndMap;
	}
	template <>
	void SceneSerializer::serialize<AnimatedMeshComponent>(YAML::Emitter& out, const AnimatedMeshComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "AnimatedMeshComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Mesh" << val.Mesh->GetHandle();
		out << YAML::Key << "Material" << val.MaterialAsset->GetHandle();

		out << YAML::Key << "BoneEntities";
		out << YAML::Value << YAML::BeginSeq << YAML::Flow;
		for (const auto boneEntity : val.BoneEntities)
		{
			out << YAML::Value << entity.GetRegistry()->get<IDComponent>(boneEntity).ID;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::serialize<AnimationComponent>(YAML::Emitter& out, const AnimationComponent& val, SceneEntity entity)
	{
		out << YAML::Key << "AnimationComponent";
		out << YAML::BeginMap;
		if (val.Controller.Raw())
		{
			out << YAML::Key << "Controller" << val.Controller->GetHandle();
		}
		else
		{
			out << YAML::Key << "Controller" << "";
		}
		out << YAML::EndMap;
	}

	template <>
	void SceneSerializer::deserialize<MeshComponent>(YAML::Node& data, SceneEntity entity)
	{
		MeshComponent& component = entity.EmplaceComponent<MeshComponent>();
		component.Mesh = AssetManager::GetAsset<Mesh>(AssetHandle(data["Mesh"].as<std::string>()));
		component.MaterialAsset = AssetManager::GetAsset<MaterialAsset>(AssetHandle(data["Material"].as<std::string>()));
	}

	template <>
	void SceneSerializer::deserialize<AnimatedMeshComponent>(YAML::Node& data, SceneEntity entity)
	{
		AnimatedMeshComponent& component = entity.EmplaceComponent<AnimatedMeshComponent>();
		component.Mesh = AssetManager::GetAsset<AnimatedMesh>(AssetHandle(data["Mesh"].as<std::string>()));
		component.MaterialAsset = AssetManager::GetAsset<MaterialAsset>(AssetHandle(data["Material"].as<std::string>()));
	}

	template <>
	void SceneSerializer::deserialize<ParticleComponent>(YAML::Node& data, SceneEntity entity)
	{
		ParticleComponent& component = entity.EmplaceComponent<ParticleComponent>();;

		component.GetSystem()->MaxParticles			   = data["MaxParticles"].as<uint32_t>();
		component.GetSystem()->Speed				   = data["Speed"].as<float>();
		component.GetSystem()->AnimationTiles		   = data["AnimationTiles"].as<glm::ivec2>();
		component.GetSystem()->AnimationStartFrame     = data["AnimationStartFrame"].as<uint32_t>();
		component.GetSystem()->AnimationCycleLength    = data["AnimationCycleLength"].as<float>();
		component.GetSystem()->EndRotation			   = data["EndRotation"].as<glm::vec3>();
		
		size_t counter = 0;
		for (auto enabled : data["ModulesEnabled"])
			component.ModulesEnabled[counter++] = enabled.as<bool>();

		{ // Emitter
			auto emitter = data["Emitter"];
			component.GetSystem()->Emitter.Shape = static_cast<EmitShape>(emitter["Shape"].as<uint32_t>());
			component.GetSystem()->Emitter.BoxMin = emitter["BoxMin"].as<glm::vec3>();
			component.GetSystem()->Emitter.BoxMax = emitter["BoxMax"].as<glm::vec3>();
			component.GetSystem()->Emitter.Radius = emitter["Radius"].as<float>();
		
			component.GetSystem()->Emitter.EmitRate = emitter["EmitRate"].as<float>();
			component.GetSystem()->Emitter.LifeTime = emitter["LifeTime"].as<float>();
			
			component.GetSystem()->Emitter.MinVelocity = emitter["MinVelocity"].as<glm::vec3>();
			component.GetSystem()->Emitter.MaxVelocity = emitter["MaxVelocity"].as<glm::vec3>();
			
			component.GetSystem()->Emitter.Size  = emitter["Size"].as<glm::vec3>();
			component.GetSystem()->Emitter.Color = emitter["Color"].as<glm::vec4>();
		
			component.GetSystem()->Emitter.LightColor		= emitter["LightColor"].as<glm::vec3>();
			component.GetSystem()->Emitter.LightRadius		= emitter["LightRadius"].as<float>();
			component.GetSystem()->Emitter.LightIntensity	= emitter["LightIntensity"].as<float>();
			component.GetSystem()->Emitter.MaxLights		= emitter["MaxLights"].as<uint32_t>();


			component.GetSystem()->Emitter.BurstInterval = emitter["BurstInterval"].as<float>();
			for (auto burst : emitter["Bursts"])
			{
				component.GetSystem()->Emitter.Bursts.push_back({
					burst["Count"].as<uint32_t>(),
					burst["Time"].as<float>(),
					burst["Probability"].as<float>()
				});	
			}
		}
	}
	template <>
	void SceneSerializer::deserialize<ParticleRenderer>(YAML::Node& data, SceneEntity entity)
	{
		auto& particleRenderer = entity.EmplaceComponent<ParticleRenderer>();
		particleRenderer.Mesh = AssetManager::GetAsset<Mesh>(AssetHandle(data["Mesh"].as<std::string>()));
		particleRenderer.MaterialAsset = AssetManager::GetAsset<MaterialAsset>(AssetHandle(data["Material"].as<std::string>()));
	}
	template <>
	void SceneSerializer::deserialize<ScriptComponent>(YAML::Node& data, SceneEntity entity)
	{
		ScriptComponent scriptComponent;
		scriptComponent.ModuleName = data["ModuleName"].as<std::string>();
		entity.AddComponent(scriptComponent);

		auto& fields = ScriptEngine::GetPublicFields(entity);
		const auto& component = entity.GetComponent<ScriptComponent>();
		for (auto& field : fields)
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
		const GUID materialHandle(data["Material"].as<std::string>());
		const GUID subTextureHandle(data["SubTexture"].as<std::string>());
		const glm::vec4 color	 = data["Color"].as<glm::vec4>();
		const uint16_t sortLayer = data["SortLayer"].as<uint16_t>();
		const bool visible	     = data["Visible"].as<bool>();

		Ref<Material> material;
		Ref<SubTexture> subTexture;
		if (AssetManager::Exist(materialHandle))
		{
			material = AssetManager::GetAsset<Material>(materialHandle);
		}
		if (AssetManager::Exist(subTextureHandle))
		{
			subTexture = AssetManager::GetAsset<SubTexture>(subTextureHandle);
		}

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
		chain.InvertNormals = data["InvertNormals"].as<bool>();

		entity.AddComponent(chain);
	}

	template <>
	void SceneSerializer::deserialize<AnimationComponent>(YAML::Node& data, SceneEntity entity)
	{
		AnimationComponent component;

		auto controllerData = data["Controller"].as<std::string>();
		if (!controllerData.empty())
		{
			AssetHandle handle(controllerData);
			component.Controller = AssetManager::TryGetAsset<AnimationController>(handle);
		}
		entity.AddComponent(component);
	}

	void SceneSerializer::deserializeEntity(YAML::Node& data,  WeakRef<Scene> scene)
	{
		GUID guid = data["Entity"].as<std::string>();
		auto& tagComponent = data["SceneTagComponent"];
		
		SceneTagComponent tag = tagComponent["Name"].as<std::string>();
		SceneEntity entity = scene->CreateEntity(tag, guid);

		auto transformComponent = data["TransformComponent"];
		if (transformComponent)
		{
			deserialize<TransformComponent>(transformComponent, entity);
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

		auto particleComponent = data["ParticleComponent"];
		if (particleComponent)
		{
			deserialize<ParticleComponent>(particleComponent, entity);
		}

		auto particleRenderer = data["ParticleRenderer"];
		if (particleRenderer)
		{
			deserialize<ParticleRenderer>(particleRenderer, entity);
		}

		auto meshComponent = data["MeshComponent"];
		if (meshComponent)
		{
			deserialize<MeshComponent>(meshComponent, entity);
		}

		auto animatedMeshComponent = data["AnimatedMeshComponent"];
		if (animatedMeshComponent)
		{
			deserialize<AnimatedMeshComponent>(animatedMeshComponent, entity);
		}

		auto animationComponent = data["AnimationComponent"];
		if (animationComponent)
		{
			deserialize<AnimationComponent>(animationComponent, entity);
		}
	}



	void SceneSerializer::Serialize(const std::string& filepath, WeakRef<Scene> scene)
	{
		YAML::Emitter out;
		
		SceneEntity sceneEntity = scene->GetSceneEntity();
		const IDComponent& idComponent = sceneEntity.GetComponent<IDComponent>();
		const Relationship& relationship = sceneEntity.GetComponent<Relationship>();
		const auto& registry = scene->GetRegistry();

		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << scene->m_Name;
		out << YAML::Key << "SceneEntity" << idComponent.ID;
		if (registry.valid(relationship.FirstChild))
			out << YAML::Key << "FirstChild" << registry.get<IDComponent>(relationship.FirstChild).ID;

		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;

		scene->m_Registry.each([&](const entt::entity ent) {

			SceneEntity entity(ent, scene.Raw());
			if (entity != scene->GetSceneEntity())
			{
				serializeEntity(out, entity);
			}
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
		fout.flush();
	}

	entt::entity FindByID(const entt::registry& reg, const GUID& guid)
	{
		auto view = reg.view<const IDComponent>();
		for (auto entity : view)
		{
			if (view.get<const IDComponent>(entity).ID == guid)
				return entity;
		}
		return entt::null;
	}

	Ref<Scene> SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		const std::string sceneName = data["Scene"].as<std::string>();
		const GUID sceneEntityGuid = data["SceneEntity"].as<std::string>();
		

		Ref<Scene> scene = Ref<Scene>::Create(sceneName, sceneEntityGuid);
		entt::registry& reg = scene->m_Registry;
		SceneEntity sceneEntity = scene->GetSceneEntity();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				SceneSerializer::deserializeEntity(entity, scene);
			}
			if (data["FirstChild"])
			{
				GUID firstChildID = data["FirstChild"].as<std::string>();
				sceneEntity.GetComponent<Relationship>().FirstChild = FindByID(reg, firstChildID);
			}
			for (auto entityData : entities)
			{
				GUID guid = entityData["Entity"].as<std::string>();
				entt::entity entity = FindByID(reg, guid);
				SceneEntity setupEntity{ entity, scene.Raw() };

				setupRelationship(entityData, setupEntity);
				auto animatedMeshComponent = entityData["AnimatedMeshComponent"];
				if (animatedMeshComponent)
					setupAnimatedMeshComponent(animatedMeshComponent, setupEntity);
			}
		}
		return scene;
	}

	void SceneSerializer::serializeEntity(YAML::Emitter& out, SceneEntity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IDComponent>().ID;

		if (entity.HasComponent<SceneTagComponent>())
		{
			serialize<SceneTagComponent>(out, entity.GetComponent<SceneTagComponent>(), entity);
		}
		if (entity.HasComponent<TransformComponent>())
		{
			serialize<TransformComponent>(out, entity.GetComponent<TransformComponent>(), entity);
		}
		if (entity.HasComponent<CameraComponent>())
		{
			serialize<CameraComponent>(out, entity.GetComponent<CameraComponent>(), entity);
		}
		if (entity.HasComponent<SpriteRenderer>())
		{
			serialize<SpriteRenderer>(out, entity.GetComponent<SpriteRenderer>(), entity);
		}
		if (entity.HasComponent<Relationship>())
		{
			SerializeRelationship(entity.m_Scene->m_Registry, out, entity.GetComponent<Relationship>());
		}
		if (entity.HasComponent<RigidBody2DComponent>())
		{
			serialize<RigidBody2DComponent>(out, entity.GetComponent<RigidBody2DComponent>(), entity);
		}
		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			serialize<BoxCollider2DComponent>(out, entity.GetComponent<BoxCollider2DComponent>(), entity);
		}
		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			serialize<CircleCollider2DComponent>(out, entity.GetComponent<CircleCollider2DComponent>(), entity);
		}
		if (entity.HasComponent<ChainCollider2DComponent>())
		{
			serialize<ChainCollider2DComponent>(out, entity.GetComponent<ChainCollider2DComponent>(), entity);
		}
		if (entity.HasComponent<ScriptComponent>())
		{
			serialize<ScriptComponent>(out, entity.GetComponent<ScriptComponent>(), entity);
		}
		if (entity.HasComponent<PointLight2D>())
		{
			serialize<PointLight2D>(out, entity.GetComponent<PointLight2D>(), entity);
		}
		if (entity.HasComponent<SpotLight2D>())
		{
			serialize<SpotLight2D>(out, entity.GetComponent<SpotLight2D>(), entity);
		}
		if (entity.HasComponent<ParticleRenderer>())
		{
			serialize<ParticleRenderer>(out, entity.GetComponent<ParticleRenderer>(), entity);
		}
		if (entity.HasComponent<ParticleComponent>())
		{
			serialize<ParticleComponent>(out, entity.GetComponent<ParticleComponent>(), entity);
		}
		if (entity.HasComponent<MeshComponent>())
		{
			serialize<MeshComponent>(out, entity.GetComponent<MeshComponent>(), entity);
		}
		if (entity.HasComponent<AnimatedMeshComponent>())
		{
			serialize<AnimatedMeshComponent>(out, entity.GetComponent<AnimatedMeshComponent>(), entity);
		}

		if (entity.HasComponent<AnimationComponent>())
		{
			serialize<AnimationComponent>(out, entity.GetComponent<AnimationComponent>(), entity);
		}

		out << YAML::EndMap; // Entity
	}


	void SceneSerializer::setupRelationship(YAML::Node& data, SceneEntity entity)
	{
		const entt::registry& reg = *entity.GetRegistry();
		Relationship& relationship = entity.GetComponent<Relationship>();
		// Remove relations created by scene
		relationship.Parent = entt::null;
		relationship.NextSibling = entt::null;
		relationship.PreviousSibling = entt::null;
		relationship.FirstChild = entt::null;

		auto relComponent = data["Relationship"];

		GUID parent = relComponent["Parent"].as<std::string>();
		relationship.Parent = FindByID(reg, parent);

		if (relComponent["NextSibling"])
		{
			GUID nextSibling = relComponent["NextSibling"].as<std::string>();
			relationship.NextSibling = FindByID(reg, nextSibling);
		}
		if (relComponent["PreviousSibling"])
		{
			GUID previousSibling = relComponent["PreviousSibling"].as<std::string>();
			relationship.PreviousSibling = FindByID(reg, previousSibling);
		}
		if (relComponent["FirstChild"])
		{
			GUID firstChild = relComponent["FirstChild"].as<std::string>();
			relationship.FirstChild = FindByID(reg, firstChild);
		}
		relationship.Depth = relComponent["Depth"].as<uint32_t>();
	}
	void SceneSerializer::setupAnimatedMeshComponent(YAML::Node& data, SceneEntity entity)
	{
		AnimatedMeshComponent& component = entity.GetComponent<AnimatedMeshComponent>();
		for (auto boneEntity : data["BoneEntities"])
		{
			entt::entity bone = FindByID(*entity.GetRegistry(), boneEntity.as<std::string>());
			component.BoneEntities.push_back(bone);
		}
	}
}
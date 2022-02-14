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

		out << YAML::Key << "Material"	<< YAML::Value << val.Material->GetHandle();
		out << YAML::Key << "SubTexture"<< YAML::Value << val.SubTexture->GetHandle();
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

		auto moduleData = val.System.GetModuleDataRead();

		out << YAML::Key << "MaxParticles" << val.System.GetMaxParticles();
		out << YAML::Key << "Speed" << val.System.GetSpeed();

		{ // Emitter
			out << YAML::Key << "Emitter";
			out << YAML::BeginMap; // Emitter

			out << YAML::Key << "EmitRate" << moduleData->Emitter.EmitRate;
			out << YAML::Key << "BurstInterval" << moduleData->Emitter.BurstInterval;

			{
				out << YAML::Key << "Bursts";
				out << YAML::Value << YAML::BeginSeq;
				for (const auto& burst : moduleData->Emitter.Bursts)
				{
					out << YAML::Key << "Burst";
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
		{ // Particle Generator
			out << YAML::Key << "ParticleGenerator";
			out << YAML::BeginMap; // ParticleGenerator

			out << YAML::Key << "Color" << moduleData->Main.Color;
			out << YAML::Key << "Size" << moduleData->Main.Size;
			out << YAML::Key << "Enabled" << moduleData->Main.Enabled;

			out << YAML::EndMap; // ParticleGenerator
		}
		{ // ParticleShapeGenerator
			out << YAML::Key << "ParticleShapeGenerator";
			out << YAML::BeginMap; // ParticleShapeGenerator

			out << YAML::Key << "Shape" << static_cast<uint32_t>(moduleData->Shape.Shape);
			out << YAML::Key << "BoxMin" << moduleData->Shape.BoxMin;
			out << YAML::Key << "BoxMax" << moduleData->Shape.BoxMax;
			out << YAML::Key << "Radius" << moduleData->Shape.Radius;
			out << YAML::Key << "Enabled" << moduleData->Shape.Enabled;

			out << YAML::EndMap; // ParticleShapeGenerator
		}
		{ // ParticleLifeGenerator
			out << YAML::Key << "ParticleLifeGenerator";
			out << YAML::BeginMap; // ParticleLifeGenerator

			out << YAML::Key << "LifeTime" << moduleData->Life.LifeTime;
			out << YAML::Key << "Enabled" << moduleData->Life.Enabled;

			out << YAML::EndMap; // ParticleLifeGenerator
		}
		{ // ParticleRandomVelocityGenerator
			out << YAML::Key << "ParticleRandomVelocityGenerator";
			out << YAML::BeginMap;

			out << YAML::Key << "MinVelocity" << moduleData->RandomVelocity.MinVelocity;
			out << YAML::Key << "MaxVelocity" << moduleData->RandomVelocity.MaxVelocity;
			out << YAML::Key << "Enabled" << moduleData->RandomVelocity.Enabled;

			out << YAML::EndMap;
		}
		{
			// LightUpdater
			out << YAML::Key << "LightUpdater";
			out << YAML::BeginMap;

			out << YAML::Key << "LightColor" << moduleData->Light.Light.Color;
			out << YAML::Key << "LightRadius" << moduleData->Light.Light.Radius;
			out << YAML::Key << "LightIntensity" << moduleData->Light.Light.Intensity;
			out << YAML::Key << "MaxLights" << moduleData->Light.MaxLights;
			out << YAML::Key << "Enabled" << moduleData->Light.Enabled;

			out << YAML::EndMap;
		}
		{
			// TextureAnimationUpdater
			out << YAML::Key << "TextureAnimationUpdater";
			out << YAML::BeginMap;

			out << YAML::Key << "Tiles" << moduleData->TextureAnim.Tiles;
			out << YAML::Key << "StartFrame" << moduleData->TextureAnim.StartFrame;
			out << YAML::Key << "CycleLength" << moduleData->TextureAnim.CycleLength;
			out << YAML::Key << "Enabled" << moduleData->TextureAnim.Enabled;
	
			out << YAML::EndMap;
		}
		{ // RotationOverLife
			out << YAML::Key << "RotationOverLife";
			out << YAML::BeginMap;

			out << YAML::Key << "EulerAngles" << moduleData->RotationOverLife.EulerAngles;
			out << YAML::Key << "CycleLength" << moduleData->RotationOverLife.CycleLength;
			out << YAML::Key << "Enabled" << moduleData->RotationOverLife.Enabled;

			out << YAML::EndMap;
		}
		out << YAML::EndMap; // Particle Component
	}

	template <>
	void SceneSerializer::deserialize<ParticleComponent>(YAML::Node& data, SceneEntity entity)
	{
		ParticleComponent component;
		auto& moduleData = component.System.GetModuleData();

		component.System.SetMaxParticles(data["MaxParticles"].as<uint32_t>());
		component.System.SetSpeed(data["Speed"].as<float>());

		{ // Emitter
			auto emitter = data["Emitter"];
			moduleData->Emitter.EmitRate = emitter["EmitRate"].as<float>();
			moduleData->Emitter.BurstInterval = emitter["BurstInterval"].as<float>();
			for (auto burst : emitter["Bursts"])
			{
				auto burstData = burst["Burst"];
				moduleData->Emitter.Bursts.push_back({
					burstData["Count"].as<uint32_t>(),
					burstData["Time"].as<float>(),
					burstData["Probability"].as<float>()
				});	
			}
		}
		{ // ParticleGenerator
			auto generator = data["ParticleGenerator"];
			moduleData->Main.Color = generator["Color"].as<glm::vec4>();
			moduleData->Main.Size = generator["Size"].as<glm::vec3>();
			moduleData->Main.Enabled = generator["Enabled"].as<bool>();
		}
		{ // ParticleShapeGenerator
			auto generator = data["ParticleShapeGenerator"];
			moduleData->Shape.Shape = static_cast<EmitShape>(generator["Shape"].as<uint32_t>());
			moduleData->Shape.BoxMin = generator["BoxMin"].as<glm::vec3>();
			moduleData->Shape.BoxMax = generator["BoxMax"].as<glm::vec3>();
			moduleData->Shape.Radius = generator["Radius"].as<float>();
			moduleData->Shape.Enabled = generator["Enabled"].as<bool>();
		}
		{ // ParticleLifeGenerator
			auto generator = data["ParticleLifeGenerator"];
			moduleData->Life.LifeTime = generator["LifeTime"].as<float>();
			moduleData->Life.Enabled = generator["Enabled"].as<bool>();
		}
		{ // ParticleRandomVelocityGenerator
			auto generator = data["ParticleRandomVelocityGenerator"];
			moduleData->RandomVelocity.MinVelocity = generator["MinVelocity"].as<glm::vec3>();
			moduleData->RandomVelocity.MaxVelocity = generator["MaxVelocity"].as<glm::vec3>();
			moduleData->RandomVelocity.Enabled = generator["Enabled"].as<bool>();
		}
		{ // LightUpdater
			auto updater = data["LightUpdater"];
			moduleData->Light.Light.Color = updater["LightColor"].as<glm::vec3>();
			moduleData->Light.Light.Radius = updater["LightRadius"].as<float>();
			moduleData->Light.Light.Intensity = updater["LightIntensity"].as<float>();
			
			moduleData->Light.MaxLights = updater["MaxLights"].as<uint32_t>();
			moduleData->Light.Enabled = updater["Enabled"].as<bool>();
		}
		{ // TextureAnimationUpdater
			auto updater = data["TextureAnimationUpdater"];
			moduleData->TextureAnim.Tiles = updater["Tiles"].as<glm::ivec2>();
			moduleData->TextureAnim.StartFrame = updater["StartFrame"].as<uint32_t>();
			moduleData->TextureAnim.CycleLength = updater["CycleLength"].as<float>();
			moduleData->TextureAnim.Enabled = updater["Enabled"].as<bool>();
		}
		{ // RotationOverLife
			auto updater = data["RotationOverLife"];
			moduleData->RotationOverLife.EulerAngles = data["EulerAngles"].as<glm::vec3>();
			moduleData->RotationOverLife.CycleLength = data["CycleLength"].as<float>();
			moduleData->RotationOverLife.Enabled = data["Enabled"].as<bool>();
		}
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

		auto particleComponent = data["ParticleComponent"];
		if (particleComponent)
		{
			deserialize<ParticleComponent>(particleComponent, entity);
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
			serializeEntity(out, entity);
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
			SerializeRelationship(entity.m_Scene->m_ECS, out, entity.GetComponent<Relationship>());
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
		out << YAML::EndMap; // Entity
	}
}
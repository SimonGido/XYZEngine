#include "stdafx.h"
#include "Serializer.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Gui/GuiContext.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"
#include "Scene.h"
#include "SceneEntity.h"
#include "AssetManager.h"

#include <filesystem>

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
	static void CreateUniqueFileName(std::string& path, const char* extension)
	{
		uint32_t counter = 0;
		if (std::filesystem::exists(path + "." + extension))
		{
			std::string numStr = std::to_string(counter);
			uint32_t digits = numStr.size();
			path += numStr;
			while (std::filesystem::exists(path + "." + extension))
			{
				path.erase(path.end() - digits);
				numStr = std::to_string(counter++);
				digits = numStr.size();
				path += numStr;
			}
		}
		path += ".";
		path += extension;
	}

	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;

		std::string_view view(path.c_str() + lastDot + 1, count);
		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}

	
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

	static enum class FieldType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		None
	};

	static FieldType FindType(const std::string& str)
	{
		char tokenComma = ',';
		size_t numCommas = std::count(str.begin(), str.end(), tokenComma);
		char tokenDot = '.';
		size_t numDots = std::count(str.begin(), str.end(), tokenDot);

		switch (numCommas)
		{
		case 0:
			if (numDots)
				return FieldType::Float;
			else
				return FieldType::Int;
		case 1:
			return FieldType::Float2;
		case 2:
			return FieldType::Float3;
		case 3:
			return FieldType::Float4;
		}

		XYZ_ASSERT(false, "Wrong type");
		return FieldType::None;
	}
	static TextureWrap IntToTextureWrap(int wrap)
	{

		if (wrap == ToUnderlying(TextureWrap::Clamp))
			return TextureWrap::Clamp;
		if (wrap == ToUnderlying(TextureWrap::Repeat))
			return TextureWrap::Repeat;


		return TextureWrap::None;
	}
	static TextureParam IntToTextureParam(int param)
	{
		if (param == ToUnderlying(TextureParam::Nearest))
			return TextureParam::Nearest;
		if (param == ToUnderlying(TextureParam::Linear))
			return TextureParam::Linear;

		return TextureParam::None;
	}


	

	static void SerializeUniformList(YAML::Emitter& out, const uint8_t* buffer, const std::vector<Uniform>& uniformList)
	{
		for (auto& uniform : uniformList)
		{
			switch (uniform.DataType)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec2*) & buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec3*) & buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec4*) & buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&buffer[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::MAT4:
				break;
			};
		}
	}




	template <>
	void Serializer::Serialize<SceneTagComponent>(YAML::Emitter& out, const SceneTagComponent& val)
	{
		out << YAML::Key << "SceneTagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << val.Name;
		out << YAML::EndMap;
	}

	template <>
	void Serializer::Serialize<TransformComponent>(YAML::Emitter& out, const TransformComponent& val)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Position" << YAML::Value << val.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << val.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << val.Scale;

		out << YAML::EndMap; // TransformComponent
	}

	template <>
	void Serializer::Serialize<CameraComponent>(YAML::Emitter& out, const CameraComponent& val)
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
	void Serializer::Serialize<SpriteRenderer>(YAML::Emitter& out, const SpriteRenderer& val)
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
	void Serializer::Serialize<PointLight2D>(YAML::Emitter& out, const PointLight2D& val)
	{
		out << YAML::Key << "PointLight2D";
		out << YAML::BeginMap; // Point Light

		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Intensity" << YAML::Value << val.Intensity;
		out << YAML::EndMap; // Point Light
	}
	template <>
	void Serializer::Serialize<RectTransform>(YAML::Emitter& out, const RectTransform& val)
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
	void Serializer::Serialize<CanvasRenderer>(YAML::Emitter& out, const CanvasRenderer& val)
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
	void Serializer::Serialize<LineRenderer>(YAML::Emitter& out, const LineRenderer& val)
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
	void Serializer::Serialize<LayoutGroup>(YAML::Emitter& out, const LayoutGroup& val)
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
	void Serializer::Serialize<Button>(YAML::Emitter& out, const Button& val)
	{
		out << YAML::Key << "Button";
		out << YAML::BeginMap;

		out << YAML::Key << "ClickColor" << YAML::Value << val.ClickColor;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
	}

	template <>
	void Serializer::Serialize<Checkbox>(YAML::Emitter& out, const Checkbox& val)
	{
		out << YAML::Key << "Checkbox";
		out << YAML::BeginMap;

		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
	}
	template <>
	void Serializer::Serialize<Slider>(YAML::Emitter& out, const Slider& val)
	{
		out << YAML::Key << "Slider";
		out << YAML::BeginMap;

		out << YAML::Key << "Value" << YAML::Value << val.Value;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;

		out << YAML::EndMap;
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
	void Serializer::Serialize<Canvas>(YAML::Emitter& out, const Canvas& val)
	{
		out << YAML::Key << "Canvas";
		out << YAML::BeginMap;

		out << YAML::Key << "Color" << YAML::Value << val.Color;

		out << YAML::EndMap;
	}


	template <>
	void Serializer::Serialize<Text>(YAML::Emitter& out, const Text& val)
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
	void Serializer::Serialize<RigidBody2DComponent>(YAML::Emitter& out, const RigidBody2DComponent& val)
	{
		out << YAML::Key << "RigidBody2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Type" << YAML::Value << ToUnderlying(val.Type);
		out << YAML::EndMap; // RigidBody2D
	}

	template <>
	void Serializer::Serialize<BoxCollider2DComponent>(YAML::Emitter& out, const BoxCollider2DComponent& val)
	{
		out << YAML::Key << "BoxCollider2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Offset" << val.Offset;
		out << YAML::Key << "Size" << val.Size;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::EndMap; // BoxCollider2D;
	}

	template <>
	void Serializer::Serialize<CircleCollider2DComponent>(YAML::Emitter& out, const CircleCollider2DComponent& val)
	{
		out << YAML::Key << "CircleCollider2D";
		out << YAML::BeginMap;

		out << YAML::Key << "Offset" << val.Offset;
		out << YAML::Key << "Radius" << val.Radius;
		out << YAML::Key << "Density" << val.Density;
		out << YAML::EndMap; // CircleCollider2D;
	}

	template <>
	void Serializer::Serialize<Entity>(YAML::Emitter& out, const Entity& entity)
	{
		if (entity.HasComponent<SceneTagComponent>())
		{
			IDComponent id = entity.GetComponent<IDComponent>();
			out << YAML::BeginMap;
			out << YAML::Key << "Entity";
			out << YAML::Value << id.ID;
			Serialize<SceneTagComponent>(out, entity.GetComponent<SceneTagComponent>());
			auto& ecs = *entity.m_ECS;
			if (ecs.Contains<SceneTagComponent>(entity))
			{
				Serialize<SceneTagComponent>(out, ecs.GetComponent<SceneTagComponent>(entity));
			}
			if (ecs.Contains<TransformComponent>(entity))
			{
				Serialize<TransformComponent>(out, ecs.GetComponent<TransformComponent>(entity));
			}
			if (ecs.Contains<CameraComponent>(entity))
			{
				Serialize<CameraComponent>(out, ecs.GetComponent<CameraComponent>(entity));
			}
			if (ecs.Contains<SpriteRenderer>(entity))
			{
				Serialize<SpriteRenderer>(out, ecs.GetComponent<SpriteRenderer>(entity));
			}
			if (ecs.Contains<Button>(entity))
			{
				Serialize<Button>(out, ecs.GetComponent<Button>(entity));
			}
			if (ecs.Contains<Checkbox>(entity))
			{
				Serialize<Checkbox>(out, ecs.GetComponent<Checkbox>(entity));
			}
			if (ecs.Contains<Slider>(entity))
			{
				Serialize<Slider>(out, ecs.GetComponent<Slider>(entity));
			}
			if (ecs.Contains<InputField>(entity))
			{
				SerializeInputField(ecs, out, ecs.GetComponent<InputField>(entity));
			}
			if (ecs.Contains<LayoutGroup>(entity))
			{
				Serialize<LayoutGroup>(out, ecs.GetComponent<LayoutGroup>(entity));
			}
			if (ecs.Contains<Relationship>(entity))
			{
				SerializeRelationship(ecs, out, ecs.GetComponent<Relationship>(entity));
			}
			if (ecs.Contains<CanvasRenderer>(entity))
			{
				Serialize<CanvasRenderer>(out, ecs.GetComponent<CanvasRenderer>(entity));
			}
			if (ecs.Contains<RectTransform>(entity))
			{
				Serialize<RectTransform>(out, ecs.GetComponent<RectTransform>(entity));
			}
			if (ecs.Contains<Canvas>(entity))
			{
				Serialize<Canvas>(out, ecs.GetComponent<Canvas>(entity));
			}
			if (ecs.Contains<Text>(entity))
			{
				Serialize<Text>(out, ecs.GetComponent<Text>(entity));
			}

			if (ecs.Contains<LineRenderer>(entity))
			{
				Serialize<LineRenderer>(out, ecs.GetComponent<LineRenderer>(entity));
			}

			if (ecs.Contains<RigidBody2DComponent>(entity))
			{
				Serialize<RigidBody2DComponent>(out, ecs.GetComponent<RigidBody2DComponent>(entity));
			}
			if (ecs.Contains<BoxCollider2DComponent>(entity))
			{
				Serialize<BoxCollider2DComponent>(out, ecs.GetComponent<BoxCollider2DComponent>(entity));
			}
			if (ecs.Contains<CircleCollider2DComponent>(entity))
			{
				Serialize<CircleCollider2DComponent>(out, ecs.GetComponent<CircleCollider2DComponent>(entity));
			}
			out << YAML::EndMap; // Entity
		}
	}

	template <>
	void Serializer::Serialize<ECSManager>(YAML::Emitter& out, const ECSManager& ecs)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "ECS";
		//out << YAML::Value << "ECS";

		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (uint32_t entity = 0; entity < ecs.GetNumberOfEntities(); ++entity)
		{
			if (ecs.GetEntitySignature(entity).any())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Entity" << YAML::Value << ecs.GetComponent<IDComponent>(entity).ID;
			
				if (ecs.Contains<SceneTagComponent>(entity))
				{
					Serialize<SceneTagComponent>(out, ecs.GetComponent<SceneTagComponent>(entity));
				}	
				if (ecs.Contains<TransformComponent>(entity))
				{
					Serialize<TransformComponent>(out, ecs.GetComponent<TransformComponent>(entity));
				}
				if (ecs.Contains<CameraComponent>(entity))
				{
					Serialize<CameraComponent>(out, ecs.GetComponent<CameraComponent>(entity));
				}
				if (ecs.Contains<SpriteRenderer>(entity))
				{
					Serialize<SpriteRenderer>(out, ecs.GetComponent<SpriteRenderer>(entity));
				}
				if (ecs.Contains<Button>(entity))
				{
					Serialize<Button>(out, ecs.GetComponent<Button>(entity));
				}
				if (ecs.Contains<Checkbox>(entity))
				{
					Serialize<Checkbox>(out, ecs.GetComponent<Checkbox>(entity));
				}
				if (ecs.Contains<Slider>(entity))
				{
					Serialize<Slider>(out, ecs.GetComponent<Slider>(entity));
				}
				if (ecs.Contains<InputField>(entity))
				{
					SerializeInputField(ecs, out, ecs.GetComponent<InputField>(entity));
				}
				if (ecs.Contains<LayoutGroup>(entity))
				{
					Serialize<LayoutGroup>(out, ecs.GetComponent<LayoutGroup>(entity));
				}
				if (ecs.Contains<Relationship>(entity))
				{
					SerializeRelationship(ecs, out, ecs.GetComponent<Relationship>(entity));
				}
				if (ecs.Contains<CanvasRenderer>(entity))
				{
					Serialize<CanvasRenderer>(out, ecs.GetComponent<CanvasRenderer>(entity));
				}
				if (ecs.Contains<RectTransform>(entity))
				{
					Serialize<RectTransform>(out, ecs.GetComponent<RectTransform>(entity));
				}
				if (ecs.Contains<Canvas>(entity))
				{
					Serialize<Canvas>(out, ecs.GetComponent<Canvas>(entity));
				}
				if (ecs.Contains<Text>(entity))
				{
					Serialize<Text>(out, ecs.GetComponent<Text>(entity));
				}
				
				if (ecs.Contains<LineRenderer>(entity))
				{
					Serialize<LineRenderer>(out, ecs.GetComponent<LineRenderer>(entity));
				}

				if (ecs.Contains<RigidBody2DComponent>(entity))
				{
					Serialize<RigidBody2DComponent>(out, ecs.GetComponent<RigidBody2DComponent>(entity));
				}
				if (ecs.Contains<BoxCollider2DComponent>(entity))
				{
					Serialize<BoxCollider2DComponent>(out, ecs.GetComponent<BoxCollider2DComponent>(entity));
				}
				if (ecs.Contains<CircleCollider2DComponent>(entity))
				{
					Serialize<CircleCollider2DComponent>(out, ecs.GetComponent<CircleCollider2DComponent>(entity));
				}
				out << YAML::EndMap; // Entity
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	
	template <>
	SpriteRenderer Serializer::Deserialize<SpriteRenderer>(YAML::Node& data, AssetManager& assetManager)
	{
		std::string materialPath = data["MaterialAssetPath"].as<std::string>();
		std::string subtexturePath = data["SubTextureAssetPath"].as<std::string>();
		glm::vec4 color = data["Color"].as<glm::vec4>();
		uint16_t sortLayer = data["SortLayer"].as<uint16_t>();

		Ref<Material> material = assetManager.GetAsset<Material>(materialPath)->GetHandle();
		Ref<SubTexture> subTexture = assetManager.GetAsset<SubTexture>(subtexturePath)->GetHandle();

		SpriteRenderer spriteRenderer(
			material,
			subTexture,
			color,
			sortLayer
		);
		return spriteRenderer;
	}

	template <>
	CameraComponent Serializer::Deserialize<CameraComponent>(YAML::Node& data, AssetManager& assetManager)
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
		perspectiveProps.PerspectiveFOV =  data["PerspectiveFOV"].as<float>();
		perspectiveProps.PerspectiveNear = data["PerspectiveNear"].as<float>();
		perspectiveProps.PerspectiveFar =  data["PerspectiveFar"].as<float>();

		orthoProps.OrthographicSize = data["OrthographicSize"].as<float>();
		orthoProps.OrthographicNear = data["OrthographicNear"].as<float>();
		orthoProps.OrthographicFar =  data["OrthographicFar"].as<float>();

		CameraComponent camera;
		camera.Camera.SetProjectionType(projectionType);
		camera.Camera.SetPerspective(perspectiveProps);
		camera.Camera.SetOrthographic(orthoProps);
		return camera;
	}

	template <>
	TransformComponent Serializer::Deserialize<TransformComponent>(YAML::Node& data, AssetManager& assetManager)
	{
		TransformComponent transform(data["Position"].as<glm::vec3>());
		
		glm::vec3 rotation = data["Rotation"].as<glm::vec3>();
		glm::vec3 scale = data["Scale"].as<glm::vec3>();

		transform.Rotation = rotation;
		transform.Scale = scale;
		return transform;
	}
	
	template <>
	RectTransform Serializer::Deserialize<RectTransform>(YAML::Node& data, AssetManager& assetManager)
	{
		RectTransform transform(data["Position"].as<glm::vec3>(), data["Size"].as<glm::vec2>());

		glm::vec3 worldPosition = data["WorldPosition"].as<glm::vec3>();
		glm::vec2 scale = data["Scale"].as<glm::vec2>();
		
		transform.WorldPosition = worldPosition;
		transform.Scale = scale;
		return transform;
	}

	template <>
	CanvasRenderer Serializer::Deserialize<CanvasRenderer>(YAML::Node& data, AssetManager& assetManager)
	{
		std::string materialPath = data["MaterialAssetPath"].as<std::string>();
		std::string subtexturePath = data["SubTextureAssetPath"].as<std::string>();
		glm::vec4 color = data["Color"].as<glm::vec4>();
		uint16_t sortLayer = data["SortLayer"].as<uint16_t>();

		Ref<Material> material = assetManager.GetAsset<Material>(materialPath)->GetHandle();
		Ref<SubTexture> subTexture = assetManager.GetAsset<SubTexture>(subtexturePath)->GetHandle();

		return CanvasRenderer(
			material, subTexture, color, Mesh(), sortLayer
		);
	}

	template <>
	PointLight2D Serializer::Deserialize<PointLight2D>(YAML::Node& data, AssetManager& assetManager)
	{
		PointLight2D light;
		light.Color = data["Color"].as<glm::vec3>();
		light.Intensity = data["Intensity"].as<float>();
		return light;
	}

	template <>
	Button Serializer::Deserialize<Button>(YAML::Node& data, AssetManager& assetManager)
	{
		glm::vec4 hooverColor = data["HooverColor"].as<glm::vec4>();
		glm::vec4 clickColor = data["ClickColor"].as<glm::vec4>();

		return Button(clickColor, hooverColor);
	}

	template <>
	Checkbox Serializer::Deserialize<Checkbox>(YAML::Node& data, AssetManager& assetManager)
	{
		glm::vec4 hooverColor = data["HooverColor"].as<glm::vec4>();
		return Checkbox(hooverColor);
	}

	template <>
	Slider Serializer::Deserialize<Slider>(YAML::Node& data, AssetManager& assetManager)
	{
		glm::vec4 hooverColor = data["HooverColor"].as<glm::vec4>();
		Slider slider(hooverColor);
		slider.Value = data["Value"].as<float>();
		return slider;
	}


	

	template <>
	LayoutGroup Serializer::Deserialize<LayoutGroup>(YAML::Node& data, AssetManager& assetManager)
	{
		LayoutGroup layout;
		layout.Padding.Left = data["LeftPadding"].as<float>();
		layout.Padding.Right = data["RightPadding"].as<float>();
		layout.Padding.Top = data["TopPadding"].as<float>();
		layout.Padding.Bottom = data["BottomPadding"].as<float>();
		layout.CellSpacing = data["CellSpacing"].as<glm::vec2>();

		return layout;
	}
	template <>
	Canvas Serializer::Deserialize<Canvas>(YAML::Node& data, AssetManager& assetManager)
	{
		glm::vec4 color = data["Color"].as<glm::vec4>();
		return Canvas(CanvasRenderMode::ScreenSpace, color);
	}

	static InputField DeserializeInputField(ECSManager& ecs, YAML::Node& data)
	{
		glm::vec4 selectColor = data["SelectColor"].as<glm::vec4>();
		glm::vec4 hooverColor = data["HooverColor"].as<glm::vec4>();
		uint32_t textEntity = ecs.FindEntity<IDComponent>(IDComponent({ data["TextEntity"].as<std::string>() }));
		InputField val(
			selectColor, hooverColor, textEntity, &ecs
		);
		return val;
	}

	static Relationship DeserializeRelationship(const ECSManager& ecs, YAML::Node& data)
	{
		Relationship relationship;
		if (data["Parent"])
		{
			std::string parent = data["Parent"].as<std::string>();
			relationship.Parent = ecs.FindEntity<IDComponent>(IDComponent( { parent } ));
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
	Text Serializer::Deserialize<Text>(YAML::Node& data, AssetManager& assetManager)
	{
		Ref<Font> font = assetManager.GetAsset<Font>(data["FontPath"].as<std::string>())->GetHandle();
		std::string source = data["Source"].as<std::string>();
		glm::vec4 color = data["Color"].as<glm::vec4>();
		TextAlignment alignment = TextAlignment::None;
		uint32_t align = data["Alignment"].as<uint32_t>();
		switch (align)
		{
		case ToUnderlying(TextAlignment::Center):
			alignment = TextAlignment::Center;
			break;
		case ToUnderlying(TextAlignment::Left):
			alignment = TextAlignment::Left;
			break;
		case ToUnderlying(TextAlignment::Right):
			alignment = TextAlignment::Right;
			break;
		}
		return Text(source, font, color, alignment);
	}

	template<>
	LineRenderer Serializer::Deserialize<LineRenderer>(YAML::Node& data, AssetManager& assetManager)
	{
		LineMesh mesh;
		glm::vec4 color = data["Color"].as<glm::vec4>();
		for (auto& seq : data["Points"])
		{
			mesh.Points.push_back(seq["Point"].as<glm::vec3>());
		}

		return LineRenderer(color, mesh);
	}

	template <>
	RigidBody2DComponent Serializer::Deserialize<RigidBody2DComponent>(YAML::Node& data, AssetManager& assetManager)
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
	BoxCollider2DComponent Serializer::Deserialize<BoxCollider2DComponent>(YAML::Node& data, AssetManager& assetManager)
	{
		BoxCollider2DComponent box;
		box.Offset = data["Offset"].as<glm::vec2>();
		box.Size = data["Size"].as<glm::vec2>();
		box.Density = data["Density"].as<float>();

		return box;
	}
	template <>
	CircleCollider2DComponent Serializer::Deserialize<CircleCollider2DComponent>(YAML::Node& data, AssetManager& assetManager)
	{
		CircleCollider2DComponent circle;
		circle.Offset = data["Offset"].as<glm::vec2>();
		circle.Radius = data["Radius"].as<float>();
		circle.Density = data["Density"].as<float>();

		return circle;
	}

	template<>
	ECSManager Serializer::Deserialize<ECSManager>(YAML::Node& data, AssetManager& assetManager)
	{
		XYZ_ASSERT(data["ECS"], "Incorrect file format");
		ECSManager ecs;
		std::string ecsName = data["ECS"].as<std::string>();
		XYZ_LOG_INFO("Deserializing ecs ", ecsName);

		auto& entities = data["Entities"];

		std::vector<uint32_t> createdEntities;
		if (entities)
		{
			for (auto& entity : entities)
			{
				uint32_t ent = ecs.CreateEntity();
				createdEntities.push_back(ent);
				GUID guid(entity["Entity"].as<std::string>());
				ecs.AddComponent<IDComponent>(ent, IDComponent(guid));

				auto tagComponent = entity["SceneTagComponent"];
				if (tagComponent)
				{
					ecs.AddComponent<SceneTagComponent>(ent, tagComponent["Name"].as<std::string>());
				}
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					ecs.AddComponent<CameraComponent>(ent, Deserialize<CameraComponent>(cameraComponent, assetManager));
				}
				auto spriteRenderer = entity["SpriteRenderer"];
				if (spriteRenderer)
				{
					ecs.AddComponent<SpriteRenderer>(ent, Deserialize<SpriteRenderer>(spriteRenderer, assetManager));
				}
				
				auto button = entity["Button"];
				if (button)
				{
					ecs.AddComponent<Button>(ent, Deserialize<Button>(button, assetManager));
				}
				auto checkbox = entity["Checkbox"];
				if (checkbox)
				{
					ecs.AddComponent<Checkbox>(ent, Deserialize<Checkbox>(checkbox, assetManager));
				}
				auto slider = entity["Slider"];
				if (slider)
				{
					ecs.AddComponent<Slider>(ent, Deserialize<Slider>(slider, assetManager));
				}
				auto layoutGroup = entity["LayoutGroup"];
				if (layoutGroup)
				{
					ecs.AddComponent<LayoutGroup>(ent, Deserialize<LayoutGroup>(layoutGroup, assetManager));
				}
				auto canvas = entity["Canvas"];
				if (canvas)
				{
					ecs.AddComponent<Canvas>(ent, Deserialize<Canvas>(canvas, assetManager));
				}
				auto rectTransform = entity["RectTransformComponent"];
				if (rectTransform)
				{
					ecs.AddComponent<RectTransform>(ent, Deserialize<RectTransform>(rectTransform, assetManager));
				}
				auto canvasRenderer = entity["CanvasRenderer"];
				if (canvasRenderer)
				{
					ecs.AddComponent<CanvasRenderer>(ent, Deserialize<CanvasRenderer>(canvasRenderer, assetManager));
				}
				
				auto pointLight = entity["PointLight2D"];
				if (pointLight)
				{
					ecs.AddComponent<PointLight2D>(ent, Deserialize<PointLight2D>(pointLight, assetManager));
				}

				auto text = entity["Text"];
				if (text)
				{
					ecs.AddComponent<Text>(ent, Deserialize<Text>(text, assetManager));
				}
				
				auto lineRenderer = entity["LineRenderer"];
				if (lineRenderer)
				{
					ecs.AddComponent<LineRenderer>(ent, Deserialize<LineRenderer>(lineRenderer, assetManager));
				}

				auto rigidBody = entity["RigidBody2D"];
				if (rigidBody)
				{
					ecs.AddComponent<RigidBody2DComponent>(ent, Deserialize<RigidBody2DComponent>(rigidBody, assetManager));
				}

				auto boxCollider = entity["BoxCollider2D"];
				if (boxCollider)
				{
					ecs.AddComponent<BoxCollider2DComponent>(ent, Deserialize<BoxCollider2DComponent>(boxCollider, assetManager));
				}

				auto circleCollider = entity["CircleCollider2D"];
				if (circleCollider)
				{
					ecs.AddComponent<CircleCollider2DComponent>(ent, Deserialize<CircleCollider2DComponent>(circleCollider, assetManager));
				}

				if (ecs.Contains<CanvasRenderer>(ent) && ecs.Contains<RectTransform>(ent))
				{
					auto& transform = ecs.GetComponent<RectTransform>(ent);
					transform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ ent, &ecs }));
				}
			}
			uint32_t counter = 0;
			for (auto entity : entities)
			{
				auto ent = createdEntities[counter++];
				auto relationship = entity["Relationship"];
				if (relationship)
				{
					ecs.AddComponent<Relationship>(ent, DeserializeRelationship(ecs, relationship));
				}

				auto inputField = entity["InputField"];
				if (inputField)
				{
					ecs.AddComponent<InputField>(ent, DeserializeInputField(ecs, inputField));
				}
			}
		}
		return ecs;
	}

	template <>
	void Serializer::Deserialize<ECSManager>(YAML::Node& data, AssetManager& assetManager, ECSManager& val)
	{
		XYZ_ASSERT(data["ECS"], "Incorrect file format");
		std::string ecsName = data["ECS"].as<std::string>();
		XYZ_LOG_INFO("Deserializing ecs ", ecsName);

		auto entities = data["Entities"];
		if (entities)
		{
			std::vector<uint32_t> createdEntities;
			for (auto entity : entities)
			{
				uint32_t ent = val.CreateEntity();
				createdEntities.push_back(ent);
				GUID guid(entity["Entity"].as<std::string>());
				val.AddComponent<IDComponent>(ent, IDComponent(guid));

				auto tagComponent = entity["SceneTagComponent"];
				if (tagComponent)
				{
					val.AddComponent<SceneTagComponent>(ent, tagComponent["Name"].as<std::string>());
				}
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					val.AddComponent<CameraComponent>(ent, Deserialize<CameraComponent>(cameraComponent, assetManager));
				}
				auto spriteRenderer = entity["SpriteRenderer"];
				if (spriteRenderer)
				{
					val.AddComponent<SpriteRenderer>(ent, Deserialize<SpriteRenderer>(spriteRenderer, assetManager));
				}

				auto button = entity["Button"];
				if (button)
				{
					val.AddComponent<Button>(ent, Deserialize<Button>(button, assetManager));
				}
				auto checkbox = entity["Checkbox"];
				if (checkbox)
				{
					val.AddComponent<Checkbox>(ent, Deserialize<Checkbox>(checkbox, assetManager));
				}
				auto slider = entity["Slider"];
				if (slider)
				{
					val.AddComponent<Slider>(ent, Deserialize<Slider>(slider, assetManager));
				}
				auto layoutGroup = entity["LayoutGroup"];
				if (layoutGroup)
				{
					val.AddComponent<LayoutGroup>(ent, Deserialize<LayoutGroup>(layoutGroup, assetManager));
				}
				auto canvas = entity["Canvas"];
				if (canvas)
				{
					val.AddComponent<Canvas>(ent, Deserialize<Canvas>(canvas, assetManager));
				}
				auto rectTransform = entity["RectTransformComponent"];
				if (rectTransform)
				{
					auto& transform = val.AddComponent<RectTransform>(ent, Deserialize<RectTransform>(rectTransform, assetManager));
				}
				auto canvasRenderer = entity["CanvasRenderer"];
				if (canvasRenderer)
				{
					val.AddComponent<CanvasRenderer>(ent, Deserialize<CanvasRenderer>(canvasRenderer, assetManager));
				}

				auto pointLight = entity["PointLight2D"];
				if (pointLight)
				{
					val.AddComponent<PointLight2D>(ent, Deserialize<PointLight2D>(pointLight, assetManager));
				}

				auto text = entity["Text"];
				if (text)
				{
					val.AddComponent<Text>(ent, Deserialize<Text>(text, assetManager));
				}

				auto rigidBody = entity["RigidBody2D"];
				if (rigidBody)
				{
					val.AddComponent<RigidBody2DComponent>(ent, Deserialize<RigidBody2DComponent>(rigidBody, assetManager));
				}

				auto boxCollider = entity["BoxCollider2D"];
				if (boxCollider)
				{
					val.AddComponent<BoxCollider2DComponent>(ent, Deserialize<BoxCollider2DComponent>(boxCollider, assetManager));
				}

				auto circleCollider = entity["CircleCollider2D"];
				if (circleCollider)
				{
					val.AddComponent<CircleCollider2DComponent>(ent, Deserialize<CircleCollider2DComponent>(circleCollider, assetManager));
				}
				
				auto lineRenderer = entity["LineRenderer"];
				if (lineRenderer)
				{
					val.AddComponent<LineRenderer>(ent, Deserialize<LineRenderer>(lineRenderer, assetManager));
				}
				if (val.Contains<CanvasRenderer>(ent) && val.Contains<RectTransform>(ent))
				{
					auto& transform = val.GetComponent<RectTransform>(ent);
					transform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ ent, &val }));
				}
			}
			uint32_t counter = 0;
			for (auto entity : entities)
			{
				auto ent = createdEntities[counter++];
				auto relationship = entity["Relationship"];
				if (relationship)
				{
					val.AddComponent<Relationship>(ent, DeserializeRelationship(val, relationship));
				}

				auto inputField = entity["InputField"];
				if (inputField)
				{
					val.AddComponent<InputField>(ent, DeserializeInputField(val, inputField));
				}
			}
		}
	}

	template <>
	void Serializer::Deserialize<SceneEntity>(YAML::Node& entity, AssetManager& assetManager, SceneEntity& val)
	{
		ECSManager& ecs = val.m_Scene->m_ECS;
		uint32_t ent = val;
		auto transformComponent = entity["TransformComponent"];
		if (transformComponent)
		{
			ecs.GetComponent<TransformComponent>(ent) = Serializer::Deserialize<TransformComponent>(transformComponent, assetManager);
		}

		auto cameraComponent = entity["CameraComponent"];
		if (cameraComponent)
		{
			ecs.AddComponent<CameraComponent>(ent, Deserialize<CameraComponent>(cameraComponent, assetManager));
		}
		auto spriteRenderer = entity["SpriteRenderer"];
		if (spriteRenderer)
		{
			ecs.AddComponent<SpriteRenderer>(ent, Deserialize<SpriteRenderer>(spriteRenderer, assetManager));
		}

		auto button = entity["Button"];
		if (button)
		{
			ecs.AddComponent<Button>(ent, Deserialize<Button>(button, assetManager));
		}
		auto checkbox = entity["Checkbox"];
		if (checkbox)
		{
			ecs.AddComponent<Checkbox>(ent, Deserialize<Checkbox>(checkbox, assetManager));
		}
		auto slider = entity["Slider"];
		if (slider)
		{
			ecs.AddComponent<Slider>(ent, Deserialize<Slider>(slider, assetManager));
		}
		auto layoutGroup = entity["LayoutGroup"];
		if (layoutGroup)
		{
			ecs.AddComponent<LayoutGroup>(ent, Deserialize<LayoutGroup>(layoutGroup, assetManager));
		}
		auto canvas = entity["Canvas"];
		if (canvas)
		{
			ecs.AddComponent<Canvas>(ent, Deserialize<Canvas>(canvas, assetManager));
		}
		auto rectTransform = entity["RectTransformComponent"];
		if (rectTransform)
		{
			ecs.AddComponent<RectTransform>(ent, Deserialize<RectTransform>(rectTransform, assetManager));
		}
		auto canvasRenderer = entity["CanvasRenderer"];
		if (canvasRenderer)
		{
			ecs.AddComponent<CanvasRenderer>(ent, Deserialize<CanvasRenderer>(canvasRenderer, assetManager));
		}

		auto pointLight = entity["PointLight2D"];
		if (pointLight)
		{
			ecs.AddComponent<PointLight2D>(ent, Deserialize<PointLight2D>(pointLight, assetManager));
		}

		auto text = entity["Text"];
		if (text)
		{
			ecs.AddComponent<Text>(ent, Deserialize<Text>(text, assetManager));
		}

		auto lineRenderer = entity["LineRenderer"];
		if (lineRenderer)
		{
			ecs.AddComponent<LineRenderer>(ent, Deserialize<LineRenderer>(lineRenderer, assetManager));
		}

		auto rigidBody = entity["RigidBody2D"];
		if (rigidBody)
		{
			ecs.AddComponent<RigidBody2DComponent>(ent, Deserialize<RigidBody2DComponent>(rigidBody, assetManager));
		}

		auto boxCollider = entity["BoxCollider2D"];
		if (boxCollider)
		{
			ecs.AddComponent<BoxCollider2DComponent>(ent, Deserialize<BoxCollider2DComponent>(boxCollider, assetManager));
		}

		auto circleCollider = entity["CircleCollider2D"];
		if (circleCollider)
		{
			ecs.AddComponent<CircleCollider2DComponent>(ent, Deserialize<CircleCollider2DComponent>(circleCollider, assetManager));
		}

		if (ecs.Contains<CanvasRenderer>(ent) && ecs.Contains<RectTransform>(ent))
		{
			auto& transform = ecs.GetComponent<RectTransform>(ent);
			transform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ ent, &ecs }));
		}
	}

	
}
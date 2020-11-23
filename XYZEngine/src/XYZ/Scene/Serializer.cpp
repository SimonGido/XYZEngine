#include "stdafx.h"
#include "Serializer.h"



#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Gui/GuiContext.h"

#include "Scene.h"
#include "Entity.h"



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
	template<>
	void Serializer::SerializeResource(const std::string& filepath, const Ref<Texture2D>& texture)
	{
		XYZ_LOG_INFO("Serializing texture ", texture->GetFilepath());
		YAML::Emitter out;
		out << YAML::BeginMap; // Texture
		out << YAML::Key << "Texture" << YAML::Value << texture->GetName();

		out << YAML::Key << "Wrap" << YAML::Value << ToUnderlying(texture->GetSpecification().Wrap);
		out << YAML::Key << "Param Min" << YAML::Value << ToUnderlying(texture->GetSpecification().MinParam);
		out << YAML::Key << "Param Max" << YAML::Value << ToUnderlying(texture->GetSpecification().MagParam);

		out << YAML::EndMap; // Texture

		std::ofstream fout(texture->GetFilepath() + ".meta");
		fout << out.c_str();
	}

	template <>
	void Serializer::SerializeResource(const std::string& filepath, const Ref<SubTexture2D>& subTexture)
	{
		XYZ_LOG_INFO("Serializing subtexture ", subTexture->GetFilepath());
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "SubTexture" << YAML::Value << subTexture->GetName();
		out << YAML::Key << "TextureAssetPath" << YAML::Value << subTexture->GetTexture()->GetFilepath();
		out << YAML::Key << "TexCoords" << YAML::Value << subTexture->GetTexCoords();
		out << YAML::EndMap;

		std::ofstream fout(subTexture->GetFilepath());
		fout << out.c_str();
	}

	template <>
	void Serializer::SerializeResource<Material>(const std::string& filepath, const Ref<Material>& material)
	{
		XYZ_LOG_INFO("Serializing material ", material->GetFilepath());
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value << material->GetName();
		out << YAML::Key << "ShaderAssetPath" << YAML::Value << material->GetShader()->GetPath();


		out << YAML::Key << "Textures";
		out << YAML::Value << YAML::BeginSeq;
		uint32_t counter = 0;
		for (auto& texture : material->GetTextures())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "TextureAssetPath";
			out << YAML::Value << texture->GetFilepath();
			out << YAML::Key << "TextureIndex";
			out << YAML::Value << counter++;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Values";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& uniform : material->GetShader()->GetVSUniformList().Uniforms)
		{
			switch (uniform.DataType)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&material->GetVSUniformBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec2*) & material->GetVSUniformBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec3*) & material->GetVSUniformBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec4*) & material->GetVSUniformBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&material->GetVSUniformBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::MAT4:
				break;
			};
		}


		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(material->GetFilepath());
		fout << out.c_str();
	}


	template <>
	void Serializer::Serialize<SceneTagComponent>(YAML::Emitter& out, const SceneTagComponent& val)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity";
		out << YAML::Value << val.Name;
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
		auto materialPath = val.Material->GetFilepath();
		if (materialPath.empty())
		{
			materialPath = "Assets/Materials/New_Material.mat";
			SerializeResource<Material>(materialPath, val.Material);
		}
		
		auto subtexturePath = val.SubTexture->GetFilepath();
		if (subtexturePath.empty())
		{
			subtexturePath = "Assets/SubTextures/New_SubTexture.subtex";
			SerializeResource<SubTexture2D>(subtexturePath, val.SubTexture);
		}
		
		out << YAML::Key << "MaterialAssetPath" << YAML::Value << materialPath;
		out << YAML::Key << "SubTextureAssetPath" << YAML::Value << subtexturePath;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "SortLayer" << YAML::Value << val.SortLayer;
		out << YAML::Key << "IsVisible" << YAML::Value << val.IsVisible;
		out << YAML::EndMap; // SpriteRenderer
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
		auto materialPath = val.Material->GetFilepath();
		if (materialPath.empty())
		{
			materialPath = "Assets/Materials/New_Material.mat";
			SerializeResource<Material>(materialPath, val.Material);
		}

		auto subtexturePath = val.SubTexture->GetFilepath();
		if (subtexturePath.empty())
		{
			subtexturePath = "Assets/SubTextures/New_SubTexture.subtex";
			SerializeResource<SubTexture2D>(subtexturePath, val.SubTexture);
		}

		out << YAML::Key << "MaterialAssetPath" << YAML::Value << materialPath;
		out << YAML::Key << "SubTextureAssetPath" << YAML::Value << subtexturePath;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "SortLayer" << YAML::Value << val.SortLayer;
		out << YAML::Key << "IsVisible" << YAML::Value << val.IsVisible;
		out << YAML::EndMap;
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

	template <>
	void Serializer::Serialize<InputField>(YAML::Emitter& out, const InputField& val)
	{
		out << YAML::Key << "InputField";
		out << YAML::BeginMap;

		out << YAML::Key << "SelectColor" << YAML::Value << val.SelectColor;
		out << YAML::Key << "HooverColor" << YAML::Value << val.HooverColor;
		out << YAML::Key << "TextEntity" << YAML::Value << val.TextEntity;

		out << YAML::EndMap;
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

			if (entity.HasComponent<TransformComponent>())
			{
				Serialize<TransformComponent>(out, entity.GetComponent<TransformComponent>());
			}
			if (entity.HasComponent<CameraComponent>())
			{
				Serialize<CameraComponent>(out, entity.GetComponent<CameraComponent>());
			}
			if (entity.HasComponent<SpriteRenderer>())
			{
				Serialize<SpriteRenderer>(out, entity.GetComponent<SpriteRenderer>());
			}
			out << YAML::EndMap; // Entity
		}
	}


	template <>
	void Serializer::SerializeResource<Scene>(const std::string& filepath, const Ref<Scene>& scene)
	{
		XYZ_LOG_INFO("Serializing scene ", scene->GetFilepath());
		Ref<Scene> sceneCopy = scene;
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << scene->GetName();

		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (auto ent : scene->m_Entities)
		{
			Entity entity(ent, sceneCopy.Raw());
			Serialize<Entity>(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	template <>
	Ref<Texture2D> Serializer::DeserializeResource<Texture2D>(const std::string& filepath)
	{
		TextureWrap wrap = TextureWrap::Clamp;
		TextureParam min = TextureParam::Linear;
		TextureParam max = TextureParam::Nearest;

		std::ifstream stream(filepath + ".meta");
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
		
			XYZ_ASSERT(data["Texture"], "Incorrect file format");
			wrap = IntToTextureWrap(data["Wrap"].as<int>());
			min = IntToTextureParam(data["Param Min"].as<int>());
			max = IntToTextureParam(data["Param Max"].as<int>());
		}
		else
		{
			XYZ_LOG_WARN("Missing texture meta data, setting default");
		}
		return Texture2D::Create(wrap, min, max, filepath);
	}

	template <>
	Ref<SubTexture2D> Serializer::DeserializeResource<SubTexture2D>(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["SubTexture"], "Incorrect file format ");

		std::string texturePath = data["TextureAssetPath"].as<std::string>();
		
		Ref<Texture2D> texture = DeserializeResource<Texture2D>(texturePath);
		glm::vec4 texCoords = data["TexCoords"].as<glm::vec4>();

		return Ref<SubTexture2D>::Create(texture, texCoords);
	}

	template <>
	Ref<Material> Serializer::DeserializeResource(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto shader = Shader::Create(data["ShaderAssetPath"].as<std::string>());
		Ref<Material> material = Ref<Material>::Create(shader);

		for (auto& seq : data["Textures"])
		{
			std::string texturepath = seq["TextureAssetPath"].as<std::string>();
			uint32_t index = seq["TextureIndex"].as<uint32_t>();
			Ref<Texture2D> texture = DeserializeResource<Texture2D>(texturepath);
			material->Set("u_Texture", texture, index);
		}

		for (auto& seq : data["Values"])
		{
			for (auto& val : seq)
			{
				std::stringstream ss;
				ss << val.second;
				auto type = FindType(ss.str());
				switch (type)
				{
				case FieldType::Float:
					material->Set(val.first.as<std::string>(), val.second.as<float>());
					break;
				case FieldType::Float2:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec2>());
					break;
				case FieldType::Float3:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec3>());
					break;
				case FieldType::Float4:
					material->Set(val.first.as<std::string>(), val.second.as<glm::vec4>());
					break;
				case FieldType::None:
					break;
				}

			}
		}
		return material;
	}

	
	template <>
	SpriteRenderer Serializer::Deserialize<SpriteRenderer>(YAML::Node& data)
	{
		std::string materialPath = data["MaterialAssetPath"].as<std::string>();
		std::string subtexturePath = data["SubTextureAssetPath"].as<std::string>();
		glm::vec4 color = data["Color"].as<glm::vec4>();
		uint16_t sortLayer = data["SortLayer"].as<uint16_t>();

		Ref<Material> material = DeserializeResource<Material>(materialPath);
		Ref<SubTexture2D> subTexture = DeserializeResource<SubTexture2D>(subtexturePath);

		SpriteRenderer spriteRenderer(
			material,
			subTexture,
			color,
			sortLayer
		);
		return spriteRenderer;
	}

	template <>
	CameraComponent Serializer::Deserialize<CameraComponent>(YAML::Node& data)
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
	TransformComponent Serializer::Deserialize<TransformComponent>(YAML::Node& data)
	{
		TransformComponent transform(data["Position"].as<glm::vec3>());
		
		glm::vec3 rotation = data["Rotation"].as<glm::vec3>();
		glm::vec3 scale = data["Scale"].as<glm::vec3>();

		transform.Rotation = rotation;
		transform.Scale = scale;
		return transform;
	}	



	template <>
	Ref<Scene> Serializer::DeserializeResource(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["Scene"], "Incorrect file format");

		std::string sceneName = data["Scene"].as<std::string>();
		XYZ_LOG_INFO("Deserializing scene ", sceneName);

		Ref<Scene> result = Ref<Scene>::Create(sceneName);
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				GUID guid;
				guid = entity["Entity"].as<std::string>();
				auto tagComponent = entity["SceneTagComponent"];
				SceneTagComponent tag = tagComponent["Name"].as<std::string>();
				Entity ent = result->CreateEntity(tag, guid);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& transform = ent.GetComponent<TransformComponent>();
					transform = Deserialize<TransformComponent>(transformComponent);
				}
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					ent.AddComponent<CameraComponent>(Deserialize<CameraComponent>(cameraComponent));
				}
				auto spriteRenderer = entity["SpriteRenderer"];
				if (spriteRenderer)
				{
					ent.AddComponent<SpriteRenderer>(Deserialize<SpriteRenderer>(spriteRenderer));
				}
				result->m_Entities.push_back(ent);
			}
		}
		return result;
	}
}
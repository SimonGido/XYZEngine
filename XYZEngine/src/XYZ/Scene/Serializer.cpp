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

	template<>
	void Serializer::Serialize(YAML::Emitter& out, const Ref<Texture2D>& texture)
	{
		XYZ_LOG_INFO("Serializing texture ", texture->GetFilepath());

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
	void Serializer::Serialize(YAML::Emitter& out, const Ref<SubTexture2D>& subTexture)
	{
		XYZ_LOG_INFO("Serializing subtexture ", subTexture->GetFilepath());

		out << YAML::BeginMap;
		out << YAML::Key << "SubTexture" << YAML::Value << subTexture->GetName();
		out << YAML::Key << "TextureAssetPath" << YAML::Value << subTexture->GetTexture()->GetFilepath();
		out << YAML::Key << "TexCoords" << YAML::Value << subTexture->GetTexCoords();
		out << YAML::EndMap;

		std::ofstream fout(subTexture->GetFilepath());
		fout << out.c_str();
	}

	template <>
	void Serializer::Serialize<Ref<Material>>(YAML::Emitter& out, const Ref<Material>& material)
	{
		XYZ_LOG_INFO("Serializing material ", material->GetFilepath());
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
		for (auto& uniform : material->GetShader()->GetUniforms())
		{
			switch (uniform.Type)
			{
			case UniformDataType::FLOAT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(float*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC2:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec2*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC3:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec3*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_VEC4:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(glm::vec4*) & material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::INT:
				out << YAML::BeginMap;
				out << YAML::Key << uniform.Name;
				out << YAML::Value << *(int*)&material->GetBuffer()[uniform.Offset];
				out << YAML::EndMap;
				break;
			case UniformDataType::FLOAT_MAT4:
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
			Serialize<Ref<Material>>(out, val.Material);
		}
		
		auto subtexturePath = val.SubTexture->GetFilepath();
		if (subtexturePath.empty())
		{
			subtexturePath = "Assets/SubTextures/New_SubTexture.subtex";
			Serialize<Ref<SubTexture2D>>(out, val.SubTexture);
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
			Serialize<Ref<Material>>(out, val.Material);
		}

		auto subtexturePath = val.SubTexture->GetFilepath();
		if (subtexturePath.empty())
		{
			subtexturePath = "Assets/SubTextures/New_SubTexture.subtex";
			Serialize<Ref<SubTexture2D>>(out, val.SubTexture);
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
	void Serializer::Serialize<Ref<Scene>>(YAML::Emitter& out, const Ref<Scene>& scene)
	{
		XYZ_LOG_INFO("Serializing scene ", scene->GetFilepath());
		Ref<Scene> sceneCopy = scene;
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

	
}
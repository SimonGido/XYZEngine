#include "stdafx.h"
#include "Serializer.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Gui/GuiContext.h"
#include "XYZ/Gui/Dockspace.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

#include "Scene.h"
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
	template<>
	void Serializer::SerializeResource(const Ref<Texture2D>& texture)
	{
		XYZ_ASSERT(!texture->GetFilepath().empty(), "Filepath is empty");
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
	void Serializer::SerializeResource(const Ref<SubTexture>& subTexture)
	{
		XYZ_ASSERT(!subTexture->GetFilepath().empty(), "Filepath is empty");
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
	void Serializer::SerializeResource<Material>(const Ref<Material>& material)
	{
		XYZ_ASSERT(!material->GetFilepath().empty(), "Filepath is empty");
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

		SerializeUniformList(out, material->GetVSUniformBuffer(), material->GetShader()->GetVSUniformList().Uniforms);
		SerializeUniformList(out, material->GetFSUniformBuffer(), material->GetShader()->GetFSUniformList().Uniforms);
		

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(material->GetFilepath());
		fout << out.c_str();
	}
	template <>
	void Serializer::SerializeResource<Font>(const Ref<Font>& font)
	{
		XYZ_ASSERT(!font->GetFilepath().empty(), "Filepath is empty");
		XYZ_LOG_INFO("Serializing font ", font->GetFilepath());
		// remove extension
		std::string pathWithoutExtension = font->GetFilepath().substr(0, font->GetFilepath().size() - 4);
		YAML::Emitter out;
		out << YAML::BeginMap; // Font
		out << YAML::Key << "Font" << YAML::Value << font->GetName();

		out << YAML::Key << "PixelSize" << YAML::Value << font->GetPixelsize();
		
		out << YAML::EndMap; // Font

		std::ofstream fout(pathWithoutExtension + ".meta");
		fout << out.c_str();
	}

	template <>
	void Serializer::SerializeResource<FrameBuffer>(const Ref<FrameBuffer>& frameBuffer)
	{
		XYZ_ASSERT(!frameBuffer->GetFilepath().empty(), "Filepath is empty");
		XYZ_LOG_INFO("Serializing FrameBuffer ", frameBuffer->GetFilepath());

		auto& specs = frameBuffer->GetSpecification();
		YAML::Emitter out;
		out << YAML::BeginMap; // FrameBuffer
		out << YAML::Key << "FrameBuffer" << YAML::Value << frameBuffer->GetName();
		
		
		out << YAML::Key << "ClearColor" << YAML::Value << specs.ClearColor;
		out << YAML::Key << "Width" << YAML::Value << specs.Width;
		out << YAML::Key << "Height" << YAML::Value << specs.Height;
		out << YAML::Key << "Samples" << YAML::Value << specs.Samples;
		out << YAML::Key << "SwapChainTarget" << YAML::Value << specs.SwapChainTarget;
	
		out << YAML::Key << "Attachments";
		out << YAML::Value << YAML::BeginSeq;
		for (auto & it : specs.Attachments.Attachments)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "TextureFormat" << YAML::Value << ToUnderlying(it.TextureFormat);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // FrameBuffer
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
	
		out << YAML::Key << "MaterialAssetPath" << YAML::Value << val.Material->GetFilepath();
		out << YAML::Key << "SubTextureAssetPath" << YAML::Value << val.SubTexture->GetFilepath();
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
		XYZ_ASSERT(!val.SubTexture->GetFilepath().empty(), "SubTexture file path is empty");
		out << YAML::Key << "CanvasRenderer";
		out << YAML::BeginMap; 

		out << YAML::Key << "MaterialAssetPath" << YAML::Value << val.Material->GetFilepath();
		out << YAML::Key << "SubTextureAssetPath" << YAML::Value << val.SubTexture->GetFilepath();
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
	void Serializer::Serialize<Relationship>(YAML::Emitter& out, const Relationship& val)
	{
		out << YAML::Key << "Relationship";
		out << YAML::BeginMap;

		out << YAML::Key << "Parent" << YAML::Value << val.Parent;
		out << YAML::Key << "NextSibling" << YAML::Value << val.NextSibling;
		out << YAML::Key << "PreviousSibling" << YAML::Value << val.PreviousSibling;
		out << YAML::Key << "FirstChild" << YAML::Value << val.FirstChild;
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
	void Serializer::Serialize<DockNodeComponent>(YAML::Emitter& out, const DockNodeComponent& val)
	{
		out << YAML::Key << "DockNodeComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Position" << YAML::Value << val.Position;
		out << YAML::Key << "Size" << YAML::Value << val.Size;
		out << YAML::Key << "Split" << YAML::Value << ToUnderlying(val.Split);
		
		if (!val.Entities.empty())
			out << YAML::Key << "Entities" << YAML::Value << val.Entities;
		out << YAML::EndMap; // DockNodeComponent
	}
	template <>
	void Serializer::Serialize<Text>(YAML::Emitter& out, const Text& val)
	{
		out << YAML::Key << "Text";
		out << YAML::BeginMap;

		out << YAML::Key << "FontPath" << YAML::Value << val.Font->GetFilepath();
		out << YAML::Key << "Source" << YAML::Value << val.Source;
		out << YAML::Key << "Color" << YAML::Value << val.Color;
		out << YAML::Key << "Alignment" << YAML::Value << ToUnderlying(val.Alignment);

		
		out << YAML::EndMap; // Text
	}

	template <>
	void Serializer::Serialize<Dockable>(YAML::Emitter& out, const Dockable& val)
	{
		out << YAML::Key << "Dockable";
		out << YAML::BeginMap;
		out << YAML::EndMap; // Dockable
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
	void Serializer::Serialize<ECSManager>(YAML::Emitter& out, const ECSManager& ecs)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "ECS";
		out << YAML::Value << "ECS";

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
					Serialize<InputField>(out, ecs.GetComponent<InputField>(entity));
				}
				if (ecs.Contains<LayoutGroup>(entity))
				{
					Serialize<LayoutGroup>(out, ecs.GetComponent<LayoutGroup>(entity));
				}
				if (ecs.Contains<Relationship>(entity))
				{
					Serialize<Relationship>(out, ecs.GetComponent<Relationship>(entity));
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
				if (ecs.Contains<DockNodeComponent>(entity))
				{
					Serialize<DockNodeComponent>(out, ecs.GetComponent<DockNodeComponent>(entity));
				}
				if (ecs.Contains<Text>(entity))
				{
					Serialize<Text>(out, ecs.GetComponent<Text>(entity));
				}
				if (ecs.Contains<Dockable>(entity))
				{
					Serialize<Dockable>(out, ecs.GetComponent<Dockable>(entity));
				}
				out << YAML::EndMap; // Entity
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	template <>
	void Serializer::SerializeResource<Scene>(const Ref<Scene>& scene)
	{
		XYZ_ASSERT(!scene->GetFilepath().empty(), "Filepath is empty");
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
			Entity entity(ent, &sceneCopy->m_ECS);
			Serialize<Entity>(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(scene->GetFilepath());
		fout << out.c_str();
	}

	template <>
	Ref<Texture2D> Serializer::DeserializeResource<Texture2D>(const std::string& filepath, AssetManager& assetManager)
	{
		std::ifstream stream(filepath + ".meta");
		TextureSpecs specs;
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
		
			XYZ_ASSERT(data["Texture"], "Incorrect file format");
			specs.Wrap = IntToTextureWrap(data["Wrap"].as<int>());
			specs.MinParam = IntToTextureParam(data["Param Min"].as<int>());
			specs.MagParam = IntToTextureParam(data["Param Max"].as<int>());
		}
		else
		{
			XYZ_LOG_WARN("Missing texture meta data, setting default");
		}

		auto ref = Texture2D::Create(specs, filepath);
		ref->SetFilepath(filepath);
		return ref;
	}

	template <>
	Ref<SubTexture> Serializer::DeserializeResource<SubTexture>(const std::string& filepath, AssetManager& assetManager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["SubTexture"], "Incorrect file format ");
	
		std::string texturePath = data["TextureAssetPath"].as<std::string>();
		
		Ref<Texture2D> texture;
		if (HasExtension(texturePath, "ttf"))
			texture = assetManager.GetAsset<Font>(texturePath)->GetHandle()->GetTexture();
		else
			texture = assetManager.GetAsset<Texture2D>(texturePath)->GetHandle();

		glm::vec4 texCoords = data["TexCoords"].as<glm::vec4>();

		auto ref = Ref<SubTexture>::Create(texture, texCoords);
		ref->SetFilepath(filepath);
		return ref;
	}

	template <>
	Ref<Material> Serializer::DeserializeResource<Material>(const std::string& filepath, AssetManager& assetManager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		auto shader = Shader::Create(data["ShaderAssetPath"].as<std::string>());
		Ref<Material> material = Ref<Material>::Create(shader);

		for (auto& seq : data["Textures"])
		{
			std::string texturePath = seq["TextureAssetPath"].as<std::string>();
			uint32_t index = seq["TextureIndex"].as<uint32_t>();
			Ref<Texture2D> texture;
			if (HasExtension(texturePath, "ttf"))
				texture = assetManager.GetAsset<Font>(texturePath)->GetHandle()->GetTexture();
			else
				texture = assetManager.GetAsset<Texture2D>(texturePath)->GetHandle();
			
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
		material->SetFilepath(filepath);
		return material;
	}
	template <>
	Ref<Font> Serializer::DeserializeResource<Font>(const std::string& filepath, AssetManager& assetManager)
	{
		std::string pathWithoutExtension = filepath.substr(0, filepath.size() - 4);
		std::ifstream stream(pathWithoutExtension + ".meta");
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["Font"], "Incorrect file format");
		uint32_t pixelSize = data["PixelSize"].as<uint32_t>();
	

		auto ref = Ref<Font>::Create(pixelSize, filepath);
		ref->GetTexture()->SetFilepath(filepath);
		ref->SetFilepath(filepath);
		return ref;
	}

	template <>
	Ref<FrameBuffer> Serializer::DeserializeResource<FrameBuffer>(const std::string& filepath, AssetManager& assetManager)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		XYZ_ASSERT(data["FrameBuffer"], "Incorect file format");
		FrameBufferSpecs specs;
		specs.ClearColor = data["ClearColor"].as<glm::vec4>();
		specs.Width = data["Width"].as<uint32_t>();
		specs.Height = data["Height"].as<uint32_t>();
		specs.Samples = data["Samples"].as<uint32_t>();
		specs.SwapChainTarget = data["SwapChainTarget"].as<uint32_t>();

		for (auto& seq : data["Attachments"])
		{
			int32_t format = seq["TextureFormat"].as<int32_t>();
			FrameBufferTextureSpecs textureSpecs;
			
			switch (format)
			{
			case ToUnderlying(FrameBufferTextureFormat::RGBA8):
				textureSpecs.TextureFormat = FrameBufferTextureFormat::RGBA8;
				break;
			case ToUnderlying(FrameBufferTextureFormat::RGBA16F):
				textureSpecs.TextureFormat = FrameBufferTextureFormat::RGBA16F;
				break;
			case ToUnderlying(FrameBufferTextureFormat::RGBA32F):
				textureSpecs.TextureFormat = FrameBufferTextureFormat::RGBA32F;
				break;
			case ToUnderlying(FrameBufferTextureFormat::RG32F):
				textureSpecs.TextureFormat = FrameBufferTextureFormat::RG32F;
				break;
			};
			specs.Attachments.Attachments.push_back(textureSpecs);
		}
		
		return FrameBuffer::Create(specs);
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

	template <>
	Relationship Serializer::Deserialize<Relationship>(YAML::Node& data, AssetManager& assetManager)
	{
		Relationship relationship;
		relationship.Parent = data["Parent"].as<uint32_t>();
		relationship.NextSibling = data["NextSibling"].as<uint32_t>();
		relationship.PreviousSibling = data["PreviousSibling"].as<uint32_t>();
		relationship.FirstChild = data["FirstChild"].as<uint32_t>();
		return relationship;
	}

	template <>
	Text Serializer::Deserialize<Text>(YAML::Node& data, AssetManager& assetManager)
	{
		Ref<Font> font = assetManager.GetAsset<Font>(data["FontPath"].as<std::string>())->GetHandle();
		std::string source = data["Source"].as<std::string>();
		glm::vec4 color = data["Color"].as<glm::vec4>();
		TextAlignment alignment;
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

	template <>
	Dockable Serializer::Deserialize<Dockable>(YAML::Node& data, AssetManager& assetManager)
	{
		return Dockable();
	}

	template <>
	DockNodeComponent Serializer::Deserialize<DockNodeComponent>(YAML::Node& data, AssetManager& assetManager)
	{
		glm::vec3 position = data["Position"].as<glm::vec3>();
		glm::vec2 size = data["Size"].as<glm::vec2>();

		DockNodeComponent dockNode(position, size);
		if (data["Entities"])
			dockNode.Entities = data["Entities"].as<std::vector<uint32_t>>();
		auto split = data["Split"].as<int32_t>();
		switch (split)
		{
		case ToUnderlying(SplitType::Vertical):
			dockNode.Split = SplitType::Vertical;
			break;
		case ToUnderlying(SplitType::Horizontal):
			dockNode.Split = SplitType::Horizontal;
			break;
		case ToUnderlying(SplitType::None):
			dockNode.Split = SplitType::None;
			break;
		};
		return dockNode;
	}

	template<>
	ECSManager Serializer::Deserialize<ECSManager>(YAML::Node& data, AssetManager& assetManager)
	{
		XYZ_ASSERT(data["ECS"], "Incorrect file format");
		ECSManager ecs;
		std::string ecsName = data["ECS"].as<std::string>();
		XYZ_LOG_INFO("Deserializing ecs ", ecsName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint32_t ent = ecs.CreateEntity();
				GUID guid;
				guid = entity["Entity"].as<std::string>();
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
				auto relationship = entity["Relationship"];
				if (relationship)
				{
					ecs.AddComponent<Relationship>(ent, Deserialize<Relationship>(relationship, assetManager));
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
				auto dockNode = entity["DockNodeComponent"];
				if (dockNode)
				{
					ecs.AddComponent<DockNodeComponent>(ent, Deserialize<DockNodeComponent>(dockNode, assetManager));
				}
				auto text = entity["Text"];
				if (text)
				{
					ecs.AddComponent<Text>(ent, Deserialize<Text>(text, assetManager));
				}
				auto dockable = entity["Dockable"];
				if (dockable)
				{
					ecs.AddComponent<Dockable>(ent, Deserialize<Dockable>(dockable, assetManager));
				}
				if (ecs.Contains<CanvasRenderer>(ent) && ecs.Contains<RectTransform>(ent))
				{
					auto& transform = ecs.GetComponent<RectTransform>(ent);
					if (ecs.Contains<Text>(ent))
					{
						transform.Execute<CanvasRendererRebuildEvent>(CanvasRendererRebuildEvent(
							{ ent,&ecs }, TextCanvasRendererRebuild()
						));
					}
					else
					{
						transform.Execute<CanvasRendererRebuildEvent>(CanvasRendererRebuildEvent(
							{ ent,&ecs }, QuadCanvasRendererRebuild()
						));
					}
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
			for (auto entity : entities)
			{
				uint32_t ent = val.CreateEntity();
				GUID guid;
				guid = entity["Entity"].as<std::string>();
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
				auto relationship = entity["Relationship"];
				if (relationship)
				{
					val.AddComponent<Relationship>(ent, Deserialize<Relationship>(relationship, assetManager));
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
				auto dockNode = entity["DockNodeComponent"];
				if (dockNode)
				{
					val.AddComponent<DockNodeComponent>(ent, Deserialize<DockNodeComponent>(dockNode, assetManager));
				}
				auto text = entity["Text"];
				if (text)
				{
					val.AddComponent<Text>(ent, Deserialize<Text>(text, assetManager));
				}
				auto dockable = entity["Dockable"];
				if (dockable)
				{
					val.AddComponent<Dockable>(ent, Deserialize<Dockable>(dockable, assetManager));
				}
				if (val.Contains<CanvasRenderer>(ent) && val.Contains<RectTransform>(ent))
				{
					auto& transform = val.GetComponent<RectTransform>(ent);
					if (val.Contains<Text>(ent))
					{
						transform.Execute<CanvasRendererRebuildEvent>(CanvasRendererRebuildEvent(
							{ ent,&val }, TextCanvasRendererRebuild()
						));
					}
					else
					{
						transform.Execute<CanvasRendererRebuildEvent>(CanvasRendererRebuildEvent(
							{ ent,&val }, QuadCanvasRendererRebuild()
						));
					}
				}
			}
		}
	}

	template <>
	Ref<Scene> Serializer::DeserializeResource(const std::string& filepath, AssetManager& assetManager)
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
					transform = Deserialize<TransformComponent>(transformComponent, assetManager);
				}
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					ent.AddComponent<CameraComponent>(Deserialize<CameraComponent>(cameraComponent, assetManager));
				}
				auto spriteRenderer = entity["SpriteRenderer"];
				if (spriteRenderer)
				{
					ent.AddComponent<SpriteRenderer>(Deserialize<SpriteRenderer>(spriteRenderer, assetManager));
				}
			}
		}
		result->SetFilepath(filepath);
		return result;
	}
}
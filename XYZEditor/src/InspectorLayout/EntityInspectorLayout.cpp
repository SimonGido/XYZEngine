#include "EntityInspectorLayout.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace XYZ {

	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;

		std::string_view view(path.c_str() + lastDot + 1, count);

		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}

	EntityInspectorLayout::EntityInspectorLayout(Entity context)
		:
		m_Context(context)
	{
		m_DefaultMaterial = Material::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		auto characterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/player_sprite.png");
		m_DefaultSubTexture = Ref<SubTexture2D>::Create(characterTexture, glm::vec2(0, 0), glm::vec2(characterTexture->GetWidth() / 8, characterTexture->GetHeight() / 3));
	}
	void EntityInspectorLayout::SetContext(Entity context)
	{
		if (context && context.HasComponent<TransformComponent>() )
		{
			auto transformComponent = context.GetComponent<TransformComponent>();
			
		
			if (context.HasComponent<SpriteRenderer>())
			{
				auto spriteRenderer = context.GetComponent<SpriteRenderer>();
				m_Sprite = spriteRenderer->SubTexture->GetName();
				m_Material = spriteRenderer->Material->GetName();
			}
			if (context.HasComponent<NativeScriptComponent>())
			{
				m_NativeScriptObject = context.GetComponent<NativeScriptComponent>()->ScriptObjectName;
			}
			auto& flags = InGui::GetWindow(m_InspectorID)->Flags;
			flags |= InGuiWindowFlag::Modified;
			flags |= InGuiWindowFlag::MenuEnabled;
		}
		m_Context = context;
	}
	void EntityInspectorLayout::AttemptSetAsset(const std::string& filepath, AssetManager& assetManager)
	{
		if (m_Context)
		{
			if (m_Context.HasComponent<SpriteRenderer>())
			{
				auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
				if (m_SpriteTextFlags & InGuiReturnType::Hoovered)
				{
					if (HasExtension(filepath, "subtex"))
					{
						auto newSubTexture = assetManager.GetAsset<SubTexture2D>(filepath);
						spriteRenderer->SubTexture = newSubTexture;
						m_Sprite = spriteRenderer->SubTexture->GetName();
					}
				}
			}
		}
	}
	bool EntityInspectorLayout::ValidExtension(const std::string& filepath)
	{
		if (m_Context)
		{
			if (m_Context.HasComponent<SpriteRenderer>())
			{
				auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
				if (m_SpriteTextFlags & InGuiReturnType::Hoovered)
				{
					if (HasExtension(filepath, "subtex"))
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	void EntityInspectorLayout::OnInGuiRender()
	{	
		if (m_Context)
		{
			if (m_Context.HasComponent<SceneTagComponent>())
			{
				auto sceneTag = m_Context.GetComponent<SceneTagComponent>();
				InGui::BeginGroup("Scene Tag Component", {0,0}, m_SceneTagOpen);
				if (m_SceneTagOpen)
				{
					InGui::TextArea("Name", sceneTag->Name, {}, { 150, 25 }, m_SceneTagModified);

					InGui::EndGroup();
					InGui::Separator();
				}
			}

			if (m_Context.HasComponent<TransformComponent>())
			{
				auto transformComponent = m_Context.GetComponent<TransformComponent>();
				InGui::BeginGroup("Transform Component", { 0,0 }, m_TransformOpen);
				if (m_TransformOpen)
				{
					float* translationPtr = (float*)&transformComponent->Translation.x;
					InGui::Float(3, "Position", glm::value_ptr(transformComponent->Translation), m_PositionLengths, {}, { 50.0f, 25.0f }, m_PositionSelected);
					if (m_PositionSelected != -1)
					{
						
					}
					InGui::Separator();

					InGui::Float(3, "Rotation", glm::value_ptr(transformComponent->Rotation), m_RotationLengths, {}, { 50.0f, 25.0f }, m_PositionSelected);
					if (m_RotationSelected != -1)
					{
						
					}
					InGui::Separator();

					InGui::Float(3, "Scale", glm::value_ptr(transformComponent->Scale), m_ScaleLengths, {}, { 50.0f, 25.0f }, m_ScaleSelected);
					if (m_ScaleSelected != -1)
					{
						
					}
					InGui::Separator();

					InGui::EndGroup();
				}
			}

			if (m_Context.HasComponent<SpriteRenderer>())
			{
				InGui::BeginGroup("Sprite Renderer", { 0,0 }, m_SpriteRendererOpen);
				if (m_SpriteRendererOpen)
				{
					auto& renderConfig = InGui::GetRenderConfiguration();
					auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[SPRITE], renderConfig.TextureID);
					m_SpriteTextFlags = InGui::TextArea("Sprite", m_Sprite, {}, { 250.0f,25.0f }, m_SpriteModified);
					if (m_SpriteModified)
					{

					}
					InGui::Separator();
					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[MATERIAL], renderConfig.TextureID);
					m_MaterialTextFlags = InGui::TextArea("Material", m_Material, {}, { 250.0f,25.0f }, m_MaterialModified);
					if (m_MaterialModified)
					{

					}
					InGui::Separator();
					InGui::Float(4, "Color", glm::value_ptr(spriteRenderer->Color), m_ColorLengths, {}, { 50.0f, 25.0f }, m_ColorSelected);
					if (m_ColorSelected != -1)
					{
						
					}
					InGui::Separator();


					InGui::Checkbox("Pick Color", {}, { 25,25 }, m_PickColor);
					if (m_PickColor)
					{
						InGui::ColorPicker4("Color", { 255,255 }, m_ColorPallete, spriteRenderer->Color);
					}
					InGui::Separator();

					InGui::EndGroup();
				}
			}

			if (m_Context.HasComponent<NativeScriptComponent>())
			{
				InGui::BeginGroup("Native Script Component", { 0,0 }, m_NativeScriptOpen);
				if (m_NativeScriptOpen)
				{
					auto nativeScript = m_Context.GetComponent<NativeScriptComponent>();
					glm::vec2 pos = { 0,0 };
					
					InGui::BeginPopup("Script", pos, { 150, 25 }, m_ScriptsOpen);
					if (m_ScriptsOpen)
					{
						AUDynArray<IObjectConstructor*> constructors;
						PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
						for (size_t i = 0; i < constructors.Size(); ++i)
						{
							if (InGui::PopupItem(constructors[i]->GetName()) & InGuiReturnType::Clicked)
							{
								ScriptableEntity* scriptableEntity = (ScriptableEntity*)NativeScriptEngine::CreateScriptObject(constructors[i]->GetName());
								auto scriptComponent = m_Context.GetComponent<NativeScriptComponent>();
								scriptComponent->ScriptObjectName = m_NativeScriptObject;
								scriptComponent->ScriptableEntity = scriptableEntity;
								scriptComponent->ScriptableEntity->Entity = m_Context;
								scriptComponent->ScriptableEntity->OnCreate();
								m_ScriptsOpen = false;
							}
						}
					}
					InGui::EndPopup();
					InGui::Separator();

					InGui::EndGroup();
				}
			}

			InGui::MenuBar("Add Component", 150, m_AddComponentOpen);
			if (m_AddComponentOpen)
			{
				if (!m_Context.HasComponent<TransformComponent>())
				{
					if (InGui::MenuItem("Add Transform", { 150,25 }) & InGuiReturnType::Clicked)
					{
						m_Context.EmplaceComponent<TransformComponent>();
						m_AddComponentOpen = false;
					}
				}
				if (!m_Context.HasComponent<SpriteRenderer>())
				{
					if (InGui::MenuItem("Add SpriteRenderer", { 150,25 }) & InGuiReturnType::Clicked)
					{
						glm::vec4 color = { 1,1,1,1 };
						m_Context.EmplaceComponent<SpriteRenderer>(
							m_DefaultMaterial,
							m_DefaultSubTexture,
							color,
							0,
							0
							);
						m_AddComponentOpen = false;
					}
				}
				if (!m_Context.HasComponent<NativeScriptComponent>())
				{
					if (InGui::MenuItem("Add Native Script", { 150,25 }) & InGuiReturnType::Clicked)
					{
						m_Context.EmplaceComponent<NativeScriptComponent>(nullptr, "");
						m_AddComponentOpen = false;
					}
				}
			}
		}
	}
}
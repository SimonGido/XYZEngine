#include "EntityInspectorLayout.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace XYZ {

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
	void EntityInspectorLayout::OnInGuiRender()
	{	
		if (m_Context)
		{
			if (m_Context.HasComponent<SceneTagComponent>())
			{
				auto sceneTag = m_Context.GetComponent<SceneTagComponent>();
				if (InGui::BeginGroup("Scene Tag Component", {0,0}, m_SceneTagOpen))
				{
					InGui::TextArea("Name", sceneTag->Name, {}, { 150, 25 }, m_SceneTagModified);

					InGui::EndGroup();
					InGui::Separator();
				}
			}

			if (m_Context.HasComponent<TransformComponent>())
			{
				auto transformComponent = m_Context.GetComponent<TransformComponent>();
				if (InGui::BeginGroup("Transform Component", { 0,0 }, m_TransformOpen))
				{
					float* translationPtr = (float*)&transformComponent->Translation.x;
					if (InGui::Float(3, "Position", glm::value_ptr(transformComponent->Translation), m_PositionLengths, {}, { 50.0f, 25.0f }, m_PositionSelected))
					{
						
					}
					InGui::Separator();

					if (InGui::Float(3, "Rotation", glm::value_ptr(transformComponent->Rotation), m_RotationLengths, {}, { 50.0f, 25.0f }, m_PositionSelected))
					{
						
					}
					InGui::Separator();

					if (InGui::Float(3, "Scale", glm::value_ptr(transformComponent->Scale), m_ScaleLengths, {}, { 50.0f, 25.0f }, m_ScaleSelected))
					{
						
					}
					InGui::Separator();

					InGui::EndGroup();
				}
			}

			if (m_Context.HasComponent<SpriteRenderer>())
			{
				if (InGui::BeginGroup("Sprite Renderer", { 0,0 }, m_SpriteRendererOpen))
				{
					auto& renderConfig = InGui::GetRenderConfiguration();
					auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[SPRITE], renderConfig.TextureID);
					if (InGui::TextArea("Sprite", m_Sprite, {}, { 250.0f,25.0f }, m_SpriteModified))
					{

					}
					InGui::Separator();
					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[MATERIAL], renderConfig.TextureID);
					if (InGui::TextArea("Material", m_Material, {}, { 250.0f,25.0f }, m_MaterialModified))
					{

					}
					InGui::Separator();
					if (InGui::Float(4, "Color", glm::value_ptr(spriteRenderer->Color), m_ColorLengths, {}, { 50.0f, 25.0f }, m_ColorSelected))
					{
						
					}
					InGui::Separator();


					if (InGui::Checkbox("Pick Color", {}, { 25,25 }, m_PickColor))
					{
						InGui::ColorPicker4("Color", { 255,255 }, m_ColorPallete, spriteRenderer->Color);
					}
					InGui::Separator();

					InGui::EndGroup();
				}
			}

			if (m_Context.HasComponent<NativeScriptComponent>())
			{
				if (InGui::BeginGroup("Native Script Component", { 0,0 }, m_NativeScriptOpen))
				{
					auto nativeScript = m_Context.GetComponent<NativeScriptComponent>();
					glm::vec2 pos = { 0,0 };
					
					if (InGui::BeginPopup("Script", pos, { 150, 25 }, m_ScriptsOpen))
					{
						AUDynArray<IObjectConstructor*> constructors;
						PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
						for (size_t i = 0; i < constructors.Size(); ++i)
						{
							if (InGui::PopupItem(constructors[i]->GetName()))
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

			if (InGui::MenuBar("Add Component", 150, m_AddComponentOpen))
			{
				if (!m_Context.HasComponent<TransformComponent>())
				{
					if (InGui::MenuItem("Add Transform", { 150,25 }))
					{
						m_Context.EmplaceComponent<TransformComponent>();
						m_AddComponentOpen = false;
					}
				}
				if (!m_Context.HasComponent<SpriteRenderer>())
				{
					if (InGui::MenuItem("Add SpriteRenderer", { 150,25 }))
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
					if (InGui::MenuItem("Add Native Script", { 150,25 }))
					{
						m_Context.EmplaceComponent<NativeScriptComponent>(nullptr, "");
						m_AddComponentOpen = false;
					}
				}
			}
		}
	}
}
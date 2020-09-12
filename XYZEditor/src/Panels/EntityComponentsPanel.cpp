#include "EntityComponentsPanel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	EntityComponentPanel::EntityComponentPanel(Entity context)
		:
		m_Context(context)
	{
		m_DefaultMaterial = Material::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		auto characterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_DefaultSubTexture = Ref<SubTexture2D>::Create(characterTexture, glm::vec2(0, 0), glm::vec2(characterTexture->GetWidth() / 8, characterTexture->GetHeight() / 3));
	
		InGui::Begin("Entity", { 0,-100 }, { 300,400 }); // To make sure it exists;
		InGui::End();
	}
	void EntityComponentPanel::SetContext(Entity context)
	{
		if (context && context.HasComponent<TransformComponent>() && (uint32_t)context != (uint32_t)m_Context)
		{
			auto transformComponent = context.GetComponent<TransformComponent>();
			glm::vec3 scale, translation, skew;
			glm::vec4 perspective;
			glm::quat orientation;
			glm::decompose(transformComponent->Transform, scale, orientation, translation, skew, perspective);


			m_XPos = std::to_string(translation.x).substr(0, 4);
			m_YPos = std::to_string(translation.y).substr(0, 4);
			m_ZPos = std::to_string(translation.z).substr(0, 4);

			m_XScale = std::to_string(scale.x).substr(0, 4);
			m_YScale = std::to_string(scale.y).substr(0, 4);
			m_ZScale = std::to_string(scale.z).substr(0, 4);

			if (context.HasComponent<SpriteRenderer>())
			{
				auto spriteRenderer = context.GetComponent<SpriteRenderer>();
				m_RColor = std::to_string(spriteRenderer->Color.x).substr(0, 4);
				m_GColor = std::to_string(spriteRenderer->Color.y).substr(0, 4);
				m_BColor = std::to_string(spriteRenderer->Color.z).substr(0, 4);
				m_AColor = std::to_string(spriteRenderer->Color.w).substr(0, 4);
			}
			if (context.HasComponent<NativeScriptComponent>())
			{
				m_NativeScriptObject = context.GetComponent<NativeScriptComponent>()->ScriptObjectName;
			}
			InGui::SetWindowFlags("entity", (Modified | MenuEnabled | Visible | EventListener));
		}
		m_Context = context;
	}
	void EntityComponentPanel::OnInGuiRender()
	{
		if (InGui::Begin("Entity", { 0,-100 }, { 300,400 }))
		{
			if (m_Context)
			{
				if (m_Context.HasComponent<SceneTagComponent>())
				{
					auto sceneTag = m_Context.GetComponent<SceneTagComponent>();
					if (InGui::BeginGroup("Scene Tag Component", m_SceneTagOpen))
					{
						InGui::TextArea("Name", sceneTag->Name, { 150,25 }, m_SceneTagModified);
					}
				}

				if (m_Context.HasComponent<TransformComponent>())
				{
					auto transformComponent = m_Context.GetComponent<TransformComponent>();
					if (InGui::BeginGroup("Transform Component", m_TransformOpen))
					{
						if (InGui::TextArea("X", m_XPos, { 50,25 }, m_XPosModified))
						{
							transformComponent->Transform[3][0] = atof(m_XPos.c_str());
						}
						if (InGui::TextArea("Y", m_YPos, { 50,25 }, m_YPosModified))
						{
							transformComponent->Transform[3][1] = atof(m_YPos.c_str());
						}
						if (InGui::TextArea("Z", m_ZPos, { 50,25 }, m_ZPosModified))
						{
							transformComponent->Transform[3][2] = atof(m_ZPos.c_str());
						}
						InGui::Text("Position", { 0.7f,0.7f });
						InGui::Separator();
						if (InGui::TextArea("X", m_XRot, { 50,25 }, m_XRotModified))
						{

						}
						if (InGui::TextArea("Y", m_YRot, { 50,25 }, m_YRotModified))
						{

						}
						if (InGui::TextArea("Z", m_ZRot, { 50,25 }, m_ZRotModified))
						{

						}
						InGui::Text("Rotation", { 0.7f,0.7f });
						InGui::Separator();
						if (InGui::TextArea("X", m_XScale, { 50,25 }, m_XScaleModified))
						{

						}
						if (InGui::TextArea("Y", m_YScale, { 50,25 }, m_YScaleModified))
						{

						}
						if (InGui::TextArea("Z", m_ZScale, { 50,25 }, m_ZScaleModified))
						{

						}
						InGui::Text("Scale", { 0.7f,0.7f });
					}
				}

				if (m_Context.HasComponent<SpriteRenderer>())
				{
					if (InGui::BeginGroup("Sprite Renderer", m_SpriteRendererOpen))
					{
						auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
						if (InGui::TextArea("R", m_RColor, { 50,25 }, m_RColorModified))
						{
							spriteRenderer->Color.x = atof(m_RColor.c_str());
						}
						if (InGui::TextArea("G", m_GColor, { 50,25 }, m_GColorModified))
						{
							spriteRenderer->Color.y = atof(m_GColor.c_str());
						}
						if (InGui::TextArea("B", m_BColor, { 50,25 }, m_BColorModified))
						{
							spriteRenderer->Color.z = atof(m_BColor.c_str());
						}
						if (InGui::TextArea("A", m_AColor, { 50,25 }, m_AColorModified))
						{
							spriteRenderer->Color.w = atof(m_AColor.c_str());
						}
						InGui::Text("Color", { 0.7f,0.7f });

						if (InGui::Checkbox("Pick Color", { 25,25 }, m_PickColor))
						{
							InGui::ColorPicker4("Color", { 255,255 }, m_ColorPallete, spriteRenderer->Color);
						}
					}
					if (m_Context.HasComponent<NativeScriptComponent>())
					{
						if (InGui::BeginGroup("Native Script Component", m_NativeScriptOpen))
						{
							auto nativeScript = m_Context.GetComponent<NativeScriptComponent>();
							if (InGui::BeginPopup("Script", { 150,25 }, m_ScriptsOpen))
							{
								AUDynArray<IObjectConstructor*> constructors;
								PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
								for (size_t i = 0; i < constructors.Size(); ++i)
								{
									if (InGui::PopupItem(constructors[i]->GetName(), { 150,25 }))
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
						}
					}
				}

				InGui::Separator();
				if (InGui::MenuBar("Add Component",150 , m_AddComponentOpen))
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
		InGui::End();
	}
}
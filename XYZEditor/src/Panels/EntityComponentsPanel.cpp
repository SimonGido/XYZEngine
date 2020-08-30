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

			if (context.HasComponent<NativeScriptComponent>())
				m_NativeScriptObject = context.GetComponent<NativeScriptComponent>()->ScriptObjectName;

			InGui::g_InContext->GetWindow("entity")->Flags |= InGui::Modified;
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
					InGui::BeginGroup();
					InGui::Text("Scene Tag Component", { 0.8f,0.8f }, {1,0.5,1,1});
					InGui::Separator();
					InGui::TextArea("Name",sceneTag->Name, { 150,25 }, m_SceneTagModified);
					InGui::Separator();
					InGui::EndGroup();
				}

				if (m_Context.HasComponent<TransformComponent>())
				{
					auto transformComponent = m_Context.GetComponent<TransformComponent>();
					InGui::BeginGroup();
					InGui::Text("Transform Component", { 0.8f,0.8f }, { 1,0.5,1,1 });
					InGui::Separator();
					InGui::Text("Position", { 0.7f,0.7f });
					if (InGui::TextArea("X",m_XPos, { 50,25 }, m_XPosModified))
					{
						transformComponent->Transform[3][0] = atof(m_XPos.c_str());
					}
					if (InGui::TextArea("Y",m_YPos, { 50,25 }, m_YPosModified))
					{
						transformComponent->Transform[3][1] = atof(m_YPos.c_str());
					}
					if (InGui::TextArea("Z",m_ZPos, { 50,25 }, m_ZPosModified))
					{
						transformComponent->Transform[3][2] = atof(m_ZPos.c_str());
					}		
					InGui::Separator();
					InGui::Text("Rotation", { 0.7f,0.7f });
					if (InGui::TextArea("X",m_XRot, { 50,25 }, m_XRotModified))
					{
						
					}
					if (InGui::TextArea("Y",m_YRot, { 50,25 }, m_YRotModified))
					{
						
					}
					if (InGui::TextArea("Z",m_ZRot, { 50,25 }, m_ZRotModified))
					{
				
					}		
					InGui::Separator();
					InGui::Text("Scale", { 0.7f,0.7f });
					if (InGui::TextArea("X",m_XScale, { 50,25 }, m_XScaleModified))
					{
						
					}
					if (InGui::TextArea("Y",m_YScale, { 50,25 }, m_YScaleModified))
					{
						
					}
					if (InGui::TextArea("Z",m_ZScale, { 50,25 }, m_ZScaleModified))
					{

					}
					InGui::Separator();
					InGui::EndGroup();
				}
				if (m_Context.HasComponent<NativeScriptComponent>())
				{
					InGui::BeginGroup();
					InGui::Text("Native Script Component", { 0.8f,0.8f }, { 1,0.5,1,1 });
					InGui::Separator();		
					auto nativeScript = m_Context.GetComponent<NativeScriptComponent>();
					if (InGui::TextArea("Script Name", m_NativeScriptObject, { 150,25 }, m_NativeScriptModified))
					{
						ScriptableEntity* scriptableEntity = (ScriptableEntity*)NativeScriptEngine::CreateScriptObject(m_NativeScriptObject);
						if (scriptableEntity)
						{
							auto scriptComponent = m_Context.GetComponent<NativeScriptComponent>();
							scriptComponent->ScriptObjectName = m_NativeScriptObject;
							scriptComponent->ScriptableEntity = scriptableEntity;
							scriptComponent->ScriptableEntity->Entity = m_Context;
							scriptComponent->ScriptableEntity->OnCreate();
						}
					}
					InGui::Separator();
					InGui::EndGroup();
				}
				if (m_Context.HasComponent<SpriteRenderer>())
				{
					InGui::BeginGroup();
					InGui::Text("Sprite Renderer", { 0.8f,0.8f }, { 1,0.5,1,1 });
					InGui::Separator();
					InGui::EndGroup();
				}
				if (InGui::BeginPopup("Add Component", { 150,25 }, m_AddComponentOpen))
				{
					if (!m_Context.HasComponent<TransformComponent>())
					{
						if (InGui::PopupItem("Add Transform", { 150,25 }))
						{
							m_Context.EmplaceComponent<TransformComponent>();
							m_AddComponentOpen = false;
						}
					}
					if (!m_Context.HasComponent<SpriteRenderer>())
					{
						if (InGui::PopupItem("Add SpriteRenderer", { 150,25 }))
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
						if (InGui::PopupItem("Add Native Script", { 150,25 }))
						{
							m_ChooseScript = true;
							m_AddComponentOpen = false;
						}
					}
				}
				InGui::EndPopup();

				if (m_ChooseScript)
				{
					if (InGui::BeginPopup("Script", { 150,25 }, m_ScriptsOpen))
					{
						AUDynArray<IObjectConstructor*> constructors;
						PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
						for (size_t i = 0; i < constructors.Size(); ++i)
						{
							if (InGui::PopupItem(constructors[i]->GetName(), { 150,25 }))
							{
								ScriptableEntity* scriptableEntity = (ScriptableEntity*)NativeScriptEngine::CreateScriptObject(constructors[i]->GetName());
								NativeScriptComponent comp(scriptableEntity, constructors[i]->GetName());
								comp.ScriptableEntity->Entity = m_Context;
								comp.ScriptableEntity->OnCreate();
								m_Context.AddComponent<NativeScriptComponent>(comp);
								m_NativeScriptObject = constructors[i]->GetName();
								m_ScriptsOpen = false;
								m_ChooseScript = false;
							}
						}
					}
					InGui::EndPopup();
				}
			}
		}
		InGui::End();
	}
}
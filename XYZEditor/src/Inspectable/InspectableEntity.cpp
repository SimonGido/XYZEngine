#include "InspectableEntity.h"

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

	InspectableEntity::InspectableEntity()

	{
		m_DefaultMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		auto characterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest, "Assets/Textures/player_sprite.png");
		m_DefaultSubTexture = Ref<SubTexture2D>::Create(characterTexture, glm::vec2(0, 0), glm::vec2((float)characterTexture->GetWidth() / 8.0f, (float)characterTexture->GetHeight() / 3.0f));
		m_DefaultMaterial->Set("u_Texture", characterTexture);
		m_DefaultMaterial->Set("u_Color", glm::vec4(1, 1, 1, 1));
	}

	void InspectableEntity::SetContext(Entity context)
	{
		if (context && context.HasComponent<TransformComponent>())
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
		}
		if (context && context.HasComponent<ParticleComponent>())
		{
			auto particleComponent = context.GetComponent<ParticleComponent>();
			auto material = particleComponent->ComputeMaterial->GetParentMaterial();
			m_MaterialUniformLengths.clear();
			m_MaterialUniformSelected.clear();
			SetupMaterialValuesLengths(material->GetShader()->GetUniforms(), m_MaterialUniformLengths, m_MaterialUniformSelected);
		}
		m_Context = context;
	}

	void InspectableEntity::AttemptSetAsset(const std::string& filepath, AssetManager& assetManager)
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
				else if (m_MaterialTextFlags & InGuiReturnType::Hoovered)
				{
					if (HasExtension(filepath, "mat"))
					{
						auto newMaterial = assetManager.GetAsset<Material>(filepath);
						spriteRenderer->Material = newMaterial;
						m_Material = newMaterial->GetName();
					}
				}
			}
		}
	}
	bool InspectableEntity::ValidExtension(const std::string& filepath)
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
				else if (m_MaterialTextFlags & InGuiReturnType::Hoovered)
				{
					if (HasExtension(filepath, "mat"))
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	void InspectableEntity::OnInGuiRender()
	{
		if (m_Context)
		{
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


			if (m_Context.HasComponent<SceneTagComponent>())
			{
				auto sceneTag = m_Context.GetComponent<SceneTagComponent>();
				InGui::BeginGroup("Scene Tag Component", { 0,0 }, m_GroupOpen[SCENE_TAG_COMPONENT]);
				if (m_GroupOpen[SCENE_TAG_COMPONENT])
				{
					InGui::TextArea("Name", sceneTag->Name, {}, { 150, 25 }, m_SceneTagModified);

					InGui::EndGroup();
					InGui::Separator();
				}
			}

			if (m_Context.HasComponent<TransformComponent>())
			{
				auto transformComponent = m_Context.GetComponent<TransformComponent>();
				InGui::BeginGroup("Transform Component", { 0,0 }, m_GroupOpen[TRANSFORM_COMPONENT]);
				if (m_GroupOpen[TRANSFORM_COMPONENT])
				{
					float* translationPtr = (float*)&transformComponent->Translation.x;
					InGui::Float(3, "Position", glm::value_ptr(transformComponent->Translation), m_PositionLengths, {}, { 50.0f, 25.0f }, m_PositionSelected);
					if (m_PositionSelected != -1)
					{

					}
					InGui::Separator();

					InGui::Float(3, "Rotation", glm::value_ptr(transformComponent->Rotation), m_RotationLengths, {}, { 50.0f, 25.0f }, m_RotationSelected);
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

			if (m_Context.HasComponent<CameraComponent>())
			{
				InGui::BeginGroup("Camera Component", { 0,0 }, m_GroupOpen[CAMERA_COMPONENT]);
				if (m_GroupOpen[CAMERA_COMPONENT])
				{
					auto& camera = m_Context.GetComponent<CameraComponent>()->Camera;
					auto& scale = m_Context.GetComponent<TransformComponent>()->Scale;
					scale.x = 1.0f;
					scale.y = 1.0f;
					InGui::BeginPopup("Projection Type", glm::vec2{}, glm::vec2{ 150, 25 }, m_CameraTypeOpen);
					if (m_CameraTypeOpen)
					{
						if (InGui::PopupItem("Orthographic") & InGuiReturnType::Clicked)
						{
							camera.SetProjectionType(CameraProjectionType::Orthographic);
							m_CameraTypeOpen = false;
						}
						if (InGui::PopupItem("Perspective") & InGuiReturnType::Clicked)
						{
							camera.SetProjectionType(CameraProjectionType::Perspective);
							m_CameraTypeOpen = false;
						}
					}
					if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
					{
						InGui::Text("Orthographic");
						InGui::Separator();
						auto props = camera.GetOrthographicProperties();
						InGui::Float(1, "Size", &props.OrthographicSize, &m_CameraSizeLength, glm::vec2{}, glm::vec2{ 150.0f,25.0f }, m_SizeSelected);
						InGui::Separator();
						InGui::Float(1, "Near Plane", &props.OrthographicNear, &m_CameraNearLength, glm::vec2{}, glm::vec2{ 150.0f,25.0f }, m_NearPlaneSelected);
						InGui::Separator();
						InGui::Float(1, "Far Plane", &props.OrthographicFar, &m_CameraFarLength, glm::vec2{}, glm::vec2{ 150.0f,25.0f }, m_FarPlaneSelected);
						InGui::Separator();
						camera.SetOrthographic(props);
					}
					else if (camera.GetProjectionType() == CameraProjectionType::Perspective)
					{
						InGui::Text("Perspective");
						InGui::Separator();
						auto props = camera.GetPerspectiveProperties();
						InGui::Slider("Field of View", glm::vec2(), glm::vec2{ 180.0f,15.0f }, props.PerspectiveFOV);
						InGui::Separator();
						InGui::Float(1, "Near Plane", &props.PerspectiveNear, &m_CameraNearLength, glm::vec2{}, glm::vec2{ 150.0f,25.0f }, m_NearPlaneSelected);
						InGui::Separator();
						InGui::Float(1, "Far Plane", &props.PerspectiveFar, &m_CameraFarLength, glm::vec2{}, glm::vec2{ 150.0f,25.0f }, m_FarPlaneSelected);
						InGui::Separator();
						camera.SetPerspective(props);
					}
				}
			}

			if (m_Context.HasComponent<SpriteRenderer>())
			{
				InGui::BeginGroup("Sprite Renderer", { 0,0 }, m_GroupOpen[SPRITE_RENDERER_COMPONENT]);
				if (m_GroupOpen[SPRITE_RENDERER_COMPONENT])
				{
					auto& renderConfig = InGui::GetRenderConfiguration();
					auto spriteRenderer = m_Context.GetComponent<SpriteRenderer>();

					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[EditorIcon::SPRITE], renderConfig.TextureID);
					m_SpriteTextFlags = InGui::TextArea("Sprite", m_Sprite, {}, { 250.0f,25.0f }, m_SpriteModified);
					if (m_SpriteModified)
					{

					}
					InGui::Separator();
					InGui::Icon({}, { 30.0f,30.0f }, renderConfig.SubTexture[EditorIcon::MATERIAL], renderConfig.TextureID);
					m_MaterialTextFlags = InGui::TextArea("Material", m_Material, {}, { 250.0f,25.0f }, m_MaterialModified);
					if (m_MaterialModified)
					{

					}
					InGui::Separator();

					InGui::Int(1, "Sort Layer", &spriteRenderer->SortLayer, &m_SortLayerLength, {}, { 50.0f,25.0f }, m_SortLayerSelected);
					InGui::Separator();

					InGui::Int(1, "Texture ID", (int32_t*)&spriteRenderer->TextureID, &m_TextureIDLength, {}, { 50.0f,25.0f }, m_TextureIDSelected);
					if (spriteRenderer->Material->GetNumberOfTextures() <= spriteRenderer->TextureID)
					{
						if (spriteRenderer->Material->GetNumberOfTextures())
						{
							spriteRenderer->TextureID = spriteRenderer->Material->GetNumberOfTextures() - 1;
						}
						else
						{
							spriteRenderer->TextureID = 0;
						}
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
			if (m_Context.HasComponent<ParticleComponent>())
			{
				InGui::BeginGroup("Particle Component", { 0,0 }, m_GroupOpen[PARTICLE_COMPONENT]);
				if (m_GroupOpen[PARTICLE_COMPONENT])
				{
					auto& renderConfig = InGui::GetRenderConfiguration();
					auto particleComponent = m_Context.GetComponent<ParticleComponent>();
				
					auto config = particleComponent->ParticleEffect->GetConfiguration();
					if (InGui::Slider("Rate", {}, { 150.0f, 15.0f }, config.Rate) & InGuiReturnType::Clicked)
					{
						particleComponent->ParticleEffect->SetConfiguration(config);
					}
					InGui::Separator();

					if (InGui::Button("Reset", { 40.0f,40.0f }) & InGuiReturnType::Clicked)
					{
						particleComponent->ParticleEffect->Restart();
					}
					InGui::Separator();
					auto& uniforms = particleComponent->ComputeMaterial->GetParentMaterial()->GetShader()->GetUniforms();
					ShowUniforms(particleComponent->ComputeMaterial->GetBuffer(), uniforms, m_MaterialUniformLengths, m_MaterialUniformSelected);
				}
			}
			if (m_Context.HasComponent<NativeScriptComponent>())
			{
				InGui::BeginGroup("Native Script Component", { 0,0 }, m_GroupOpen[NATIVE_SCRIPT_COMPONENT]);
				if (m_GroupOpen[NATIVE_SCRIPT_COMPONENT])
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
			
		}
	}
	void InspectableEntity::OnUpdate(Timestep ts)
	{
	}
	void InspectableEntity::OnEvent(Event& event)
	{
	}
}
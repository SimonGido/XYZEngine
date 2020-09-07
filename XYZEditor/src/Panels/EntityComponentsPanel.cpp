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
			//InGui::GetWindow("entity")->Flags |= InGui::Modified;
		}
		m_Context = context;
	}
	void EntityComponentPanel::OnInGuiRender()
	{	
		
	}
}
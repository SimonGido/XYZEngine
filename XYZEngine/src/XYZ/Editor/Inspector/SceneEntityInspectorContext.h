#pragma once
#include "InspectorContext.h"
#include "InspectorEditable.h"
#include "XYZ/Editor/Panels/ScenePanel.h"
#include "Components/ComponentInspectors.h"

namespace XYZ {
	namespace Editor {
		class SceneEntityInspectorContext : public InspectorContext
		{
		public:
			SceneEntityInspectorContext();

			virtual void OnImGuiRender() override;

			void SetContext(SceneEntity context);
		
		private:
			std::vector<InspectorEditable*> m_InspectorEditables;

			SceneEntity						m_Context;
			glm::vec2						m_IconSize;

			Ref<Material>					m_DefaultMaterial;
			Ref<SubTexture>					m_DefaultSubTexture;


		private:
			CameraInspector		          m_CameraInspector;
			PointLight2DInspector         m_PointLight2DInspector;
			SpotLight2DInspector          m_SpotLight2DInspector;
			ParticleComponentGPUInspector m_ParticleGPUInspector;
			SceneTagInspector			  m_SceneTagInspector;
			ScriptComponentInspector	  m_ScriptComponentInspector;
			TransformInspector			  m_TransformInspector;
			SpriteRendererInspector		  m_SpriteRendererInspector;
			RigidBody2DInspector		  m_RigidBodyInspector;
			BoxCollider2DInspector		  m_BoxCollider2DInspector;
			CircleCollider2DInspector	  m_CircleCollider2DInspector;
			ChainCollider2DInspector	  m_ChainCollider2DInspector;
		};
	}
}
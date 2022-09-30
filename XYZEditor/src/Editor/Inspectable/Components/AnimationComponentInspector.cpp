#include "stdafx.h"
#include "AnimationComponentInspector.h"


#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Renderer/Renderer2D.h"

#include "XYZ/ImGui/ImGui.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {
		AnimationComponentInspector::AnimationComponentInspector()
			:
			Inspectable("AnimationInspector")
		{
		}
		bool AnimationComponentInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<AnimationComponent>("Animation Component", m_Context, [&](auto& component) {

				
			});
		}
		void AnimationComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
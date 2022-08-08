#include "stdafx.h"
#include "AnimationInspector.h"


#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Renderer/Renderer2D.h"

#include "XYZ/ImGui/ImGui.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {
		AnimationInspector::AnimationInspector()
			:
			Inspectable("AnimationInspector")
		{
		}
		bool AnimationInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<AnimationComponent>("Animation", m_Context, [&](auto& component) {

				
			});
		}
		void AnimationInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
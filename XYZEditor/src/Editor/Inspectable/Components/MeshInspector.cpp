#include "stdafx.h"
#include "MeshInspector.h"


#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		MeshInspector::MeshInspector()
			:
			Inspectable("Mesh Inspector")
		{
		}
		bool MeshInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<MeshComponent>("Mesh Component", m_Context, [&](auto& component) {

			});
		}
		void MeshInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
		AnimatedMeshInspector::AnimatedMeshInspector()
			:
			Inspectable("Animated Mesh Inspector")
		{
		}
		bool AnimatedMeshInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<AnimatedMeshComponent>("Animated Mesh Component", m_Context, [&](auto& component) {

			});
		}
		void AnimatedMeshInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
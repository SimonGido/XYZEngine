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

				std::string name = "Controller Invalid";
				if (component.Controller.Raw())
					name = "Controller Valid";
				ImGui::InputText("##ControllerName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);


				char* assetPath = nullptr;
				if (UI::DragDropTarget("AssetDragAndDrop", &assetPath))
				{
					std::filesystem::path path(assetPath);
					if (AssetManager::Exist(path))
					{
						auto& metadata = AssetManager::GetMetadata(path);
						if (metadata.Type == AssetType::AnimationController)
						{
							Ref<AnimationController> asset = AssetManager::GetAsset<AnimationController>(metadata.Handle);
							component.Controller = asset;
						}
					}
				}
				
				bool playing = component.Playing;
				if (ImGui::Checkbox("Playing", &playing))
				{
					component.Playing = playing;
				}

			});
		}
		void AnimationComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
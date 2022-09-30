#include "stdafx.h"
#include "AnimationControllerInspector.h"

#include "XYZ/Asset/AssetManager.h"

#include "XYZ/ImGui/ImGui.h"

#include <imgui/imgui.h>

namespace XYZ {
	namespace Editor {
		AnimationControllerInspector::AnimationControllerInspector()
			:
			Inspectable("AnimationControllerInspector")
		{
		}
		bool AnimationControllerInspector::OnEditorRender()
		{
			m_ControllerAsset = AssetManager::GetAsset<AnimationController>(m_ControllerAsset->GetHandle());
			
			ImGui::Text("Skeleton");
			handleSkeleton();

			return false;
		}
		void AnimationControllerInspector::SetAsset(const Ref<Asset>& asset)
		{
			m_ControllerAsset = asset.As<AnimationController>();
		}
		void AnimationControllerInspector::handleSkeleton()
		{
			std::string name = "";
			if (m_ControllerAsset->GetSkeleton().Raw())
				name = Utils::GetFilenameWithoutExtension(m_ControllerAsset->GetSkeleton()->GetFilePath());
			ImGui::InputText("##SkeletonName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);

			char* assetPath = nullptr;
			if (UI::DragDropTarget("AssetDragAndDrop", &assetPath))
			{
				std::filesystem::path path(assetPath);
				if (AssetManager::Exist(path))
				{
					auto& metadata = AssetManager::GetMetadata(path);
					if (metadata.Type == AssetType::Skeleton)
					{
						Ref<SkeletonAsset> asset = AssetManager::GetAsset<SkeletonAsset>(metadata.Handle);
						m_ControllerAsset->SetSkeletonAsset(asset);
						AssetManager::Serialize(m_ControllerAsset->GetHandle());
					}
				}
			}
		}
	}
}
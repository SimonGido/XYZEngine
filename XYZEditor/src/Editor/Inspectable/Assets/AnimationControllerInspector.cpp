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

			ImGui::Text("Animations");
			handleAnimations();

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
				name = m_ControllerAsset->GetSkeleton()->GetFilePath();
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
		void AnimationControllerInspector::handleAnimations()
		{
			auto& animations = m_ControllerAsset->GetAnimationStates();

			uint32_t index = 0;
			for (auto& animation : animations)
			{
				
			}
			if (ImGui::Button("Add Animation"))
			{
				
			}
			char* assetPath = nullptr;
			if (UI::DragDropTarget("AssetDragAndDrop", &assetPath))
			{
				std::filesystem::path path(assetPath);
				if (AssetManager::Exist(path))
				{
					auto& metadata = AssetManager::GetMetadata(path);
					if (metadata.Type == AssetType::Animation)
					{
						Ref<AnimationAsset> asset = AssetManager::GetAsset<AnimationAsset>(metadata.Handle);
						m_ControllerAsset->AddState(asset->GetName(), asset);
						AssetManager::Serialize(m_ControllerAsset->GetHandle());
					}
				}
			}
		}
	}
}
#include "stdafx.h"
#include "AnimationControllerInspector.h"

#include "XYZ/Asset/AssetManager.h"

#include "XYZ/ImGui/ImGui.h"

#include "Editor/EditorHelper.h"

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

			ImGui::Text("States");
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
				name = AssetManager::GetMetadata(m_ControllerAsset->GetHandle()).FilePath.string();

			ImGui::InputText("##SkeletonName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);
			
			Ref<SkeletonAsset> asset;
			if (EditorHelper::AssetDragAcceptor(asset))
			{
				m_ControllerAsset->SetSkeletonAsset(asset);
				AssetManager::Serialize(m_ControllerAsset->GetHandle());
			}
		}
		void AnimationControllerInspector::handleAnimations()
		{
			auto& animations = m_ControllerAsset->GetAnimationStates();

			if (ImGui::BeginTable("##AnimationStates", 2, ImGuiTableFlags_SizingStretchProp))
			{
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				for (size_t i = 0; i < m_ControllerAsset->GetAnimationStates().size(); ++i)
				{
					const std::string rowName = "AnimationState" + std::to_string(i);
					UI::TableRow(rowName.c_str(),
						[&]() 
						{
							UI::ScopedTableColumnAutoWidth scoped(2, lineHeight);

							Ref<AnimationAsset> animation = m_ControllerAsset->GetAnimationStates()[i];
							std::string stateName = m_ControllerAsset->GetStateNames()[i];
							if (UI::InputText("##StateName", stateName))
							{
								m_ControllerAsset->SetState(i, stateName, animation);
								AssetManager::Serialize(m_ControllerAsset->GetHandle());
							}

							ImGui::SameLine();

							const std::string& filepath = animation->GetFilePath();
							ImGui::InputText("##AnimationAsset", (char*)filepath.c_str(), filepath.size(), ImGuiInputTextFlags_ReadOnly);

							Ref<AnimationAsset> asset;
							if (EditorHelper::AssetDragAcceptor(asset))
							{
								m_ControllerAsset->SetState(i, stateName, asset);
								AssetManager::Serialize(m_ControllerAsset->GetHandle());
							}
						}
					);
				}
				ImGui::EndTable();
			}
			if (ImGui::Button("Add State"))
			{
				
			}

			Ref<AnimationAsset> asset;
			if (EditorHelper::AssetDragAcceptor(asset))
			{
				m_ControllerAsset->AddState(asset->GetName(), asset);
				AssetManager::Serialize(m_ControllerAsset->GetHandle());
			}
		}
	}
}
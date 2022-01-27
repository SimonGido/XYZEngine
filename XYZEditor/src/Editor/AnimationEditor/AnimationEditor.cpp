#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/Asset/AssetSerializer.h"

#include "Editor/EditorHelper.h"
#include "EditorLayer.h"


#include <imgui.h>
#include <ImSequencer.h>


namespace XYZ {
	namespace Editor {

		AnimationEditor::AnimationEditor(std::string name)
			:
			EditorPanel(std::move(name)),
			m_ButtonSize(25.0f),
			m_SelectedEntry(-1),
			m_FirstFrame(0),
			m_CurrentFrame(0),
			m_Expanded(true),
			m_Playing(false),
			m_SplitterWidth(300.0f)
		{
			for (const auto name : ReflectedClasses::sc_ClassNames)
				m_SequencerItemTypes.push_back(std::string(name));
		}
		void AnimationEditor::SetContext(const Ref<Animator>& context)
		{
			m_Context = context;
			m_Animation = m_Context->GetAnimation();
			onEntitySelected();
		}
		void AnimationEditor::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			if (m_Playing && m_Context.Raw())
			{
				m_Animation->Update(ts);
				m_CurrentFrame = static_cast<int>(m_Animation->GetCurrentFrame());
			
				handleEditKeyValues();
			}
		}

		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Context.Raw() && m_Scene.Raw())
				{
					if (m_AnimatorEntity != m_Context->GetSceneEntity())
						onEntitySelected();
					setSequencersBoundaries();
					if (m_AnimatorEntity.IsValid())
					{
						const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

						keySelectionActions();
						UI::SplitterV(&m_SplitterWidth, "##PropertySection", "##TimelineSection",
							[&]() { propertySection(); },
							[&]() { timelineSection(); });
					}
				}
			}
			ImGui::End();
		}
		void AnimationEditor::onEntitySelected()
		{
			m_AnimatorEntity = m_Context->GetSceneEntity();
			createSequencers();
			m_ClassMap.BuildMap(m_AnimatorEntity);
		}
		void AnimationEditor::createSequencers()
		{
			m_Sequencers.clear();
			
			m_Sequencers.emplace_back(m_AnimatorEntity.GetComponent<SceneTagComponent>().Name, m_SequencerItemTypes);
			
			std::vector<Entity> tree = m_AnimatorEntity.GetComponent<Relationship>().GetTree(*m_AnimatorEntity.GetECS());
			for (auto entity : tree)
			{
				SceneEntity ent(entity, m_AnimatorEntity.GetScene());
				m_Sequencers.emplace_back(ent.GetComponent<SceneTagComponent>().Name, m_SequencerItemTypes);
			}
		}
		void AnimationEditor::setSequencersBoundaries()
		{
			for (auto& seq : m_Sequencers)
			{
				seq.FrameMin = m_FrameMin;
				seq.FrameMax = m_FrameMax;
			}
		}
		void AnimationEditor::propertySection()
		{
			UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			if (EditorButton("Beginning", m_ButtonSize, ED::MediaBeginningIcon))
			{

			}
			ImGui::SameLine();
			if (EditorButton("PrevKeyFrame", m_ButtonSize, ED::MediaNextIcon, true))
			{

			}
			ImGui::SameLine();			
			if (EditorButton("Play", m_ButtonSize, ED::MediaPlayIcon))
			{
				m_Playing = !m_Playing;
			}
			ImGui::SameLine();
			if (EditorButton("NextKeyFrame", m_ButtonSize, ED::MediaNextIcon))
			{

			}
			ImGui::SameLine();
			if (EditorButton("End", m_ButtonSize, ED::MediaBeginningIcon, true))
			{

			}


			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("AddProperty");
			}
			if (ImGui::BeginPopup("AddProperty"))
			{
				std::string selectedEntity, selectedClass, selectedVariable;
				if (m_ClassMap.OnImGuiRender(selectedEntity, selectedClass, selectedVariable))
				{
					m_ClassMap.Execute(selectedClass, selectedVariable, [&](auto classIndex, auto variableIndex) 
					{
						auto reflClass = ReflectedClasses::Get<classIndex.value>();
						auto& val = reflClass.Get<variableIndex.value>(m_AnimatorEntity.GetComponentFromReflection(reflClass));
						addReflectedProperty<variableIndex.value>(reflClass, val, selectedEntity, selectedVariable);
						m_Context->UpdateAnimationEntities();
					});
				}
				ImGui::EndPopup();
			}


			if (ImGui::BeginTable("##ControlTable", 2, ImGuiTableFlags_SizingFixedSame))
			{			
				UI::TableRow("FrameMax",
					[]() { ImGui::Text("Frame Max"); },
					[&]() 
					{
						if (ImGui::DragInt("##Frame Max", &m_FrameMax, 0.5f, m_FrameMin, INT_MAX, "%d"))
							m_Animation->SetNumFrames(m_FrameMax);
					});

				UI::TableRow("FrameMin",
					[]() { ImGui::Text("Frame Min"); },
					[&]()
					{
						if (ImGui::DragInt("##Frame Min", &m_FrameMin, 0.5f, 0, m_FrameMax, "%d"))
							m_FrameMin = std::max(m_FrameMin, 0);
					});

				UI::TableRow("Frame",
					[]() { ImGui::Text("Frame"); },
					[&]()
					{
						if (ImGui::DragInt("##Frame", &m_CurrentFrame, 0.5f, m_FrameMin, m_FrameMax, "%d"))
							m_Animation->SetCurrentFrame(m_CurrentFrame);
					});
				UI::TableRow("FPS",
					[]() { ImGui::Text("FPS"); },
					[&]()
					{
						int fps = static_cast<int>(m_Animation->GetFrequency());
						if (ImGui::DragInt("##FPS", &fps, 0.5f, 0, 0, "%d"))
							m_Animation->SetFrequency(static_cast<uint32_t>(fps));
					});

				
				ImGui::EndTable();
			}
		}
		void AnimationEditor::timelineSection()
		{
			if (ImGui::Button("Add Key"))
			{
				handleAddKey();
			}
		
			for (auto& seq : m_Sequencers)
			{
				int currentFrame = m_CurrentFrame;		
				const int sequenceOptions = ImSequencer::SEQUENCER_EDIT_STARTEND;

				ImSequencer::Sequencer(&seq, &currentFrame, &m_Expanded, &m_SelectedEntry, &m_FirstFrame, sequenceOptions);
				m_CurrentFrame = std::max(currentFrame, 0);
			}
		}
		void AnimationEditor::handleEditKeyEndFrames()
		{
			//auto& selection = m_Sequencer.GetSelection();
			//if (selection.ItemIndex != -1)
			//{
			//	const auto& item = m_Sequencer.GetItem(selection.ItemIndex);
			//	const char* itemTypeName = m_Sequencer.GetItemTypeName(item.Type);
			//	for (const auto& p : selection.Points)
			//	{
			//		const auto& line = m_Sequencer.GetLine(selection.ItemIndex, p.curveIndex);
			//		uint32_t endFrame = static_cast<uint32_t>(line.Points[p.pointIndex].x);
			//
			//		m_ClassMap.Execute(itemTypeName, line.Name, [&](auto classIndex, auto variableIndex) {
			//
			//			auto reflClass = ReflectedClasses::Get<classIndex.value>();
			//			Entity entity = m_SelectedEntity.ID();
			//			if (!item.Path.empty())
			//			{
			//				const auto& relation = m_SelectedEntity.GetComponent<Relationship>();
			//				entity = relation.FindByName(*m_SelectedEntity.GetECS(), item.Path);
			//			}
			//				
			//			//auto& val = reflClass.Get<variableIndex.value>(item.Entity.GetComponentFromReflection(reflClass));
			//			//auto prop = getProperty(reflClass, val, item.Path, reflClass.sc_VariableNames[variableIndex.value]);
			//			//if (prop)
			//			//	prop->SetKeyFrame(endFrame, p.pointIndex);
			//		});
			//	}
			//}
		}
	
		void AnimationEditor::handleEditKeyValues()
		{
			//if (m_SelectedEntry != -1)
			//{
			//	const int itemType = m_Sequencer.GetItemItemType(m_SelectedEntry);
			//	const char* itemTypeName = m_Sequencer.GetItemTypeName(itemType);
			//	if (const auto line = m_Sequencer.GetSelectedLine(m_SelectedEntry))
			//	{
			//		size_t classIndex, variableIndex;
			//		//if (getClassAndVariableFromNames(itemTypeName, line->Name.c_str(), classIndex, variableIndex))
			//		//{
			//		//	Reflect::For([&](auto j) {
			//		//		if (j.value == classIndex)
			//		//		{
			//		//			auto reflClass = ReflectedClasses::Get<j.value>();
			//		//			Reflect::For([&](auto i) {
			//		//				if (i.value == variableIndex)
			//		//				{
			//		//					//auto& val = reflClass.Get<i.value>(m_SelectedEntity.GetComponentFromReflection(reflClass));
			//		//					//editKeyValue(reflClass, m_SelectedEntity, m_CurrentFrame, val, reflClass.GetVariables()[i.value]);
			//		//				}
			//		//			}, std::make_index_sequence<reflClass.sc_NumVariables>());
			//		//		}
			//		//	}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			//		//}
			//	}
			//}
		}
		void AnimationEditor::handleAddKey()
		{
			//if (m_SelectedEntry != -1)
			//{
			//	const int itemType = m_Sequencer.GetItemItemType(m_SelectedEntry);
			//	const char* itemTypeName = m_Sequencer.GetItemTypeName(itemType);
			//	if (const auto line = m_Sequencer.GetSelectedLine(m_SelectedEntry))
			//	{
			//		m_Sequencer.AddKey(m_SelectedEntry, m_CurrentFrame);
			//		size_t classIndex, variableIndex;
			//		//if (getClassAndVariableFromNames(itemTypeName, line->Name.c_str(), classIndex, variableIndex))
			//		//{
			//		//	Reflect::For([&](auto j) {
			//		//		if (j.value == classIndex)
			//		//		{
			//		//			auto reflClass = ReflectedClasses::Get<j.value>();
			//		//			Reflect::For([&](auto i) {
			//		//				if (i.value == variableIndex)
			//		//				{
			//		//					//auto& val = reflClass.Get<i.value>(m_SelectedEntity.GetComponentFromReflection(reflClass));
			//		//					//addKeyToProperty(reflClass, m_SelectedEntity, m_CurrentFrame, val, reflClass.GetVariables()[i.value]);
			//		//				}
			//		//			}, std::make_index_sequence<reflClass.sc_NumVariables>());
			//		//		}
			//		//	}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			//		//}
			//		m_Sequencer.ClearSelection();
			//	}
			//}
		}

		void AnimationEditor::keySelectionActions()
		{
			
			{
				//ImGui::OpenPopup("KeyActions");
				//if (ImGui::BeginPopup("KeyActions"))
				//{
				//	if (ImGui::MenuItem("Copy Keys"))
				//	{
				//		m_Sequencer.Copy();
				//		ImGui::CloseCurrentPopup();
				//		m_OpenSelectionActions = false;
				//	}
				//
				//	const bool copied = !m_Sequencer.GetCopy().Points.empty();
				//	if (!copied)
				//	{
				//		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				//		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				//	}
				//	if (ImGui::MenuItem("Paste Keys"))
				//	{
				//		ImGui::CloseCurrentPopup();
				//		m_OpenSelectionActions = false;
				//	}
				//
				//	if (!copied)
				//	{
				//		ImGui::PopItemFlag();
				//		ImGui::PopStyleVar();
				//	}
				//
				//	if (ImGui::MenuItem("Delete Keys"))
				//	{
				//		m_Sequencer.DeleteSelectedPoints();
				//		ImGui::CloseCurrentPopup();
				//		m_OpenSelectionActions = false;
				//	}
				//	ImGui::EndPopup();					
				//}
				
			}
		}

		AnimationSequencer* AnimationEditor::findSequencer(const std::string& path)
		{
			for (auto& seq : m_Sequencers)
			{
				if (seq.GetName() == path)
					return &seq;
			}
			XYZ_ASSERT(false, "");
			return nullptr;
		}

		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::mat4>(uint32_t frame, glm::mat4& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec4>(uint32_t frame, glm::vec4& value, const std::string& valName)
		{
			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec3>(uint32_t frame, glm::vec3& value, const std::string& valName)
		{
			bool result = false;
			ImGui::Text(valName.c_str());
			if (ImGui::InputFloat("X", &value.x))
				result = true;
			if (ImGui::InputFloat("Y", &value.y))
				result = true;
			if (ImGui::InputFloat("Z", &value.z))
				result = true;

			return result;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec2>(uint32_t frame, glm::vec2& value, const std::string& valName)
		{
			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<float>(uint32_t frame, float& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<uint32_t>(uint32_t frame, uint32_t& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<bool>(uint32_t frame, bool& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<void*>(uint32_t frame, void*& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<Ref<Material>>(uint32_t frame, Ref<Material>& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<Ref<SubTexture>>(uint32_t frame, Ref<SubTexture>& value, const std::string& valName)
		{

			return false;
		}
	}
}

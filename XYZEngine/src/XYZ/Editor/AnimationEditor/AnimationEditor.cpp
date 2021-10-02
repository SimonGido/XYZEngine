#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Editor/EditorHelper.h"

#include <imgui.h>
#include <ImSequencer.h>

namespace XYZ {
	namespace Editor {

		namespace Helper {
			template <typename ...Args>
			static bool IntControl(const char* text, const char* id, int32_t& value, Args&& ...args)
			{
				ImGui::Text(text, std::forward<Args>(args)...);
				ImGui::SameLine();
				return ImGui::InputInt(id, &value);
			}


			template <typename T>
			static void AddToClassMap(Reflection<T> refl, const SceneEntity& entity, const Ref<Animation>& anim, AnimationEditor::ClassMap& classMap)
			{
				if (entity.HasComponent<T>())
				{
					std::vector<std::string> tmpVariables;
					const char* className = refl.sc_ClassName;
					for (const auto& variable : refl.GetVariables())
					{
						if (!anim->PropertyHasVariable(className, variable.c_str(), entity))
							tmpVariables.push_back(variable);
					}
					if (!tmpVariables.empty())
						classMap[refl.sc_ClassName].VariableNames = std::move(tmpVariables);
				}
			}
		}
		AnimationEditor::AnimationEditor()
			:
			m_SelectedEntry(-1),
			m_FirstFrame(0),
			m_CurrentFrame(0),
			m_Expanded(true),
			m_Playing(false),
			m_PropertySectionWidth(300.0f),
			m_TimelineSectionWidth(300.0f)
		{
			m_Sequencer.m_FrameMin = 0;
			m_Sequencer.m_FrameMax = 60;
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;
			m_Context->SetNumFrames(static_cast<uint32_t>(m_Sequencer.m_FrameMax));
		}
		void AnimationEditor::SetScene(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			if (m_Playing && m_Context.Raw())
			{
				m_Context->Update(ts);
				m_CurrentFrame = static_cast<int>(m_Context->GetCurrentFrame());
			}
		}

		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Context.Raw() && m_Scene.Raw())
				{
					SceneEntity selectedEntity = m_Scene->GetSelectedEntity();
					if (m_SelectedEntity != selectedEntity)
					{
						m_SelectedEntity = selectedEntity;
						buildClassMap(m_SelectedEntity);
					}
					if (selectedEntity.IsValid())
					{
						ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
						m_TimelineSectionWidth = viewportPanelSize.x - m_PropertySectionWidth - 5.0f;

						EditorHelper::DrawSplitter(false, 5.0f, &m_PropertySectionWidth, &m_TimelineSectionWidth, 50.0f, 50.0f);
						propertySection();
						ImGui::SameLine();
						timelineSection();
						
						if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
						{
						}
					}
				}
			}
			ImGui::End();
		}
		void AnimationEditor::propertySection()
		{
			if (ImGui::BeginChild("##PropertySection", ImVec2(m_PropertySectionWidth, 0)))
			{
				if (ImGui::Button("Play"))
				{
					m_Playing = !m_Playing;
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Key"))
				{
					handleAddKey();
				}
				ImGui::SameLine();


				if (ImGui::Button("Add Property"))
					ImGui::OpenPopup("AddProperty");

				if (ImGui::BeginPopup("AddProperty"))
				{
					size_t classIndex, variableIndex;
					if (getClassAndVariable(classIndex, variableIndex))
					{
						Reflect::For([&](auto j) {
							if (j.value == classIndex)
							{
								auto reflClass = ReflectedClasses::Get<j.value>();
								Reflect::For([&](auto i) {
									if (i.value == variableIndex)
									{
										auto& val = reflClass.Get<i.value>(m_SelectedEntity.GetComponentFromReflection(reflClass));
										addReflectedProperty(reflClass, m_SelectedEntity, val,
											reflClass.GetVariables()[i.value]
										);
									}
								}, std::make_index_sequence<reflClass.sc_NumVariables>());
							}
						}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
					}
					ImGui::EndPopup();
				}
				handleEditKeyValues();
			}
			ImGui::EndChild();
		}
		void AnimationEditor::timelineSection()
		{
			if (ImGui::BeginChild("##TimelineSection", ImVec2(m_TimelineSectionWidth, 0)))
			{
				int currentFrame = m_CurrentFrame;

				ImGui::PushItemWidth(130);
				Helper::IntControl("Frame Min", "##Frame Min", m_Sequencer.m_FrameMin);

				ImGui::SameLine();
				if (Helper::IntControl("Frame Max", "##Frame Max", m_Sequencer.m_FrameMax))
				{
					m_Context->SetNumFrames(static_cast<uint32_t>(m_Sequencer.m_FrameMax));
				}
				ImGui::SameLine();
				if (Helper::IntControl("Frame", "##Frame", currentFrame))
				{
					m_CurrentFrame = std::max(currentFrame, 0);
					m_Context->SetCurrentFrame(m_CurrentFrame);
				}
				ImGui::SameLine();
				int fps = static_cast<int>(m_Context->GetFrequency());
				if (Helper::IntControl("FPS", "##FPS", fps))
				{
					m_Context->SetFrequency(static_cast<uint32_t>(fps));
				}


				ImGui::PopItemWidth();
				int sequenceOptions = ImSequencer::SEQUENCER_EDIT_STARTEND;
				
				ImSequencer::Sequencer(&m_Sequencer, &currentFrame, &m_Expanded, &m_SelectedEntry, &m_FirstFrame, sequenceOptions);
				m_CurrentFrame = std::max(currentFrame, 0);
			}
			ImGui::EndChild();
		}
		void AnimationEditor::handleEditKeyEndFrames()
		{
			auto& selection = m_Sequencer.GetSelection();
			if (selection.ItemIndex != -1)
			{
				const auto& item = m_Sequencer.GetItem(selection.ItemIndex);
				const char* itemTypeName = m_Sequencer.GetItemTypeName(item.Type);
				for (const auto& p : selection.Points)
				{
					const auto& line = m_Sequencer.GetLine(selection.ItemIndex, p.curveIndex);
					uint32_t endFrame = static_cast<uint32_t>(line.Points[p.pointIndex].x);
					size_t classIndex, variableIndex;
					if (getClassAndVariableFromNames(itemTypeName, line.Name.c_str(), classIndex, variableIndex))
					{
						Reflect::For([&](auto j) {
							if (j.value == classIndex)
							{
								auto reflClass = ReflectedClasses::Get<j.value>();
								Reflect::For([&](auto i) {
									if (i.value == variableIndex)
									{
										auto& val = reflClass.Get<i.value>(item.Entity.GetComponentFromReflection(reflClass));
										auto prop = getProperty(reflClass, val, item.Entity, reflClass.GetVariables()[i.value]);
										if (prop)
											prop->SetKeyFrame(endFrame, p.pointIndex);
									}
								}, std::make_index_sequence<reflClass.sc_NumVariables>());
							}
						}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
					}
				}
			}
		}
	
		void AnimationEditor::handleEditKeyValues()
		{
			if (m_SelectedEntry != -1)
			{
				int itemType = m_Sequencer.GetItemItemType(m_SelectedEntry);
				const char* itemTypeName = m_Sequencer.GetItemTypeName(itemType);
				if (auto line = m_Sequencer.GetSelectedLine(m_SelectedEntry))
				{
					size_t classIndex, variableIndex;
					if (getClassAndVariableFromNames(itemTypeName, line->Name.c_str(), classIndex, variableIndex))
					{
						Reflect::For([&](auto j) {
							if (j.value == classIndex)
							{
								auto reflClass = ReflectedClasses::Get<j.value>();
								Reflect::For([&](auto i) {
									if (i.value == variableIndex)
									{
										auto& val = reflClass.Get<i.value>(m_SelectedEntity.GetComponentFromReflection(reflClass));
										editKeyValue(reflClass, m_SelectedEntity, m_CurrentFrame, val, reflClass.GetVariables()[i.value]);
									}
								}, std::make_index_sequence<reflClass.sc_NumVariables>());
							}
						}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
					}
				}
			}
		}
		void AnimationEditor::handleAddKey()
		{
			if (m_SelectedEntry != -1)
			{
				int itemType = m_Sequencer.GetItemItemType(m_SelectedEntry);
				const char* itemTypeName = m_Sequencer.GetItemTypeName(itemType);
				if (auto line = m_Sequencer.GetSelectedLine(m_SelectedEntry))
				{
					m_Sequencer.AddKey(m_SelectedEntry, m_CurrentFrame);
					size_t classIndex, variableIndex;
					if (getClassAndVariableFromNames(itemTypeName, line->Name.c_str(), classIndex, variableIndex))
					{
						Reflect::For([&](auto j) {
							if (j.value == classIndex)
							{
								auto reflClass = ReflectedClasses::Get<j.value>();
								Reflect::For([&](auto i) {
									if (i.value == variableIndex)
									{
										auto& val = reflClass.Get<i.value>(m_SelectedEntity.GetComponentFromReflection(reflClass));
										addKeyToProperty(reflClass, m_SelectedEntity, m_CurrentFrame, val, reflClass.GetVariables()[i.value]);
									}
								}, std::make_index_sequence<reflClass.sc_NumVariables>());
							}
						}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
					}
					m_Sequencer.ClearSelection();
				}
			}
		}

		void AnimationEditor::buildClassMap(const SceneEntity& entity)
		{
			m_ClassMap.clear();
			if (entity)
			{
				Reflect::For([&](auto j) {
					auto reflClass = ReflectedClasses::Get<j.value>();
					Helper::AddToClassMap(reflClass, entity, m_Context, m_ClassMap);

				}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			}
		}

		template <typename T>
		static int64_t FindIndex(const T& container, std::string_view name)
		{
			int64_t result = -1;
			for (const auto& it : container)
			{
				if (it == name)
					return result + 1;
				result++;
			}
			return result;
		}

		bool AnimationEditor::getClassAndVariable(size_t& classIndex, size_t& variableIndex)
		{
			for (auto& [className, classData] : m_ClassMap)
			{
				if (ImGui::BeginMenu(className.c_str()))
				{
					auto& variables = classData.VariableNames;
					for (auto it = variables.begin(); it != variables.end(); ++it)
					{
						if (ImGui::MenuItem(it->c_str()))
						{
							classIndex = FindIndex(ReflectedClasses::GetClasses(), className);
							Reflect::For([&](auto j) {
								if (j.value == classIndex)
								{
									auto reflClass = ReflectedClasses::Get<j.value>();
									variableIndex = FindIndex(reflClass.GetVariables(), *it);
								}
							}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());


							variables.erase(it);
							ImGui::EndMenu();
							return true;
						}
					}
					ImGui::EndMenu();
				}
			}
			return false;
		}
		bool AnimationEditor::getClassAndVariableFromNames(std::string_view className, std::string_view variableName, size_t& classIndex, size_t& variableIndex)
		{
			int64_t tmpClassIndex = FindIndex(ReflectedClasses::GetClasses(), className);
			int64_t tmpVariableIndex = -1;
			Reflect::For([&](auto j) {
				if (j.value == tmpClassIndex)
				{
					auto reflClass = ReflectedClasses::Get<j.value>();
					tmpVariableIndex = FindIndex(reflClass.GetVariables(), variableName);
				}
			}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());

			if (tmpClassIndex != -1 && tmpVariableIndex != -1)
			{
				classIndex = tmpClassIndex;
				variableIndex = tmpVariableIndex;
				return true;
			}
			return false;
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
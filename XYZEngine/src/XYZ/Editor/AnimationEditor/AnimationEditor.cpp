#include "stdafx.h"
#include "AnimationEditor.h"

namespace XYZ {
	namespace Editor {
		AnimationEditor::AnimationEditor(const std::string& filepath)
			:
			EditorUI(filepath)
		{
			bUITimeline& timeline = bUI::GetUI<bUITimeline>(GetName(), "Timeline");
			bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>(GetName(), "Scrollbox");
			scrollbox.FitParent = true;

			timeline.Rows.push_back({ "First" });
			timeline.Rows.push_back({ "Second" });
			timeline.Rows.push_back({ "Third" });
			timeline.Rows.push_back({ "Fifth" });
			timeline.Rows.push_back({ "Sixth" });
			timeline.Rows.push_back({ "Seventh" });
			timeline.Rows.push_back({ "Eighth" });

			timeline.TimePoints.push_back({ 0, 0.3f });
			timeline.TimePoints.push_back({ 1, 0.45f });
			timeline.TimePoints.push_back({ 2, 0.33f });
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			bUIAllocator& allocator = bUI::GetAllocator(GetName());
			bUIWindow& window = *allocator.GetElement<bUIWindow>("Animation Editor");
			bUITimeline& timeline = *allocator.GetElement<bUITimeline>("Timeline");
			timeline.Size.x = window.Size.x;
		}
		void AnimationEditor::OnReload()
		{
		}
		void AnimationEditor::SetupUI()
		{
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;
		}
		void AnimationEditor::OnEvent(Event& event)
		{
		}
	}
}
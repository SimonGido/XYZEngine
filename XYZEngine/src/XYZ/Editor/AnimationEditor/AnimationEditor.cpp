#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Core/Input.h"

namespace XYZ {
	namespace Editor {
		AnimationEditor::AnimationEditor(const std::string& filepath)
			:
			EditorUI(filepath),
			m_Play(false)
		{
			
			m_Layout = { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {2, 1}, true };
			SetupUI();		
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			updateLayout();
			if (m_Context.Raw())
			{
				if (m_Play)
					m_Context->Update(ts);
			}
		}
		void AnimationEditor::OnReload()
		{
			SetupUI();
		}
		void AnimationEditor::SetupUI()
		{
			bUIAllocator& allocator = bUI::GetAllocator(GetName());
			bUIScrollbox& scrollbox = *allocator.GetElement<bUIScrollbox>("Scrollbox");
			bUITimeline& timeline = *allocator.GetElement<bUITimeline>("Timeline");
			bUI::SetupLayout(allocator, scrollbox, m_Layout);

			timeline.Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					bUITimeline& casted = static_cast<bUITimeline&>(element);
					if (m_Context.Raw())
					{
						auto [mx, my] = Input::GetMousePosition();
						bUIAllocator& alloc = bUI::GetAllocator(GetName());
						bUIWindow& window = *alloc.GetElement<bUIWindow>("Animation Editor");
						bUIScrollbox& scroll = *alloc.GetElement<bUIScrollbox>("Scrollbox");

						glm::vec2 absolutePosition = casted.GetAbsolutePosition();
						float offsetX = absolutePosition.x - window.GetAbsolutePosition().x;
						float segmentLength = (casted.SplitTime / casted.Length) * casted.Zoom;
						float mouseDiffX = (mx - absolutePosition.x - offsetX + scroll.Offset.x);
						float currentTime = (mouseDiffX / segmentLength) * casted.SplitTime;
						m_Context->SetCurrentTime(currentTime);
					}
				}
			});

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
			
			scrollbox.FitParent = true;
			scrollbox.EnableScroll = true;

			bUIImage& playImage = *allocator.GetElement<bUIImage>("Play");
			playImage.FitParent = false;
			playImage.ImageSubTexture = bUI::GetConfig().GetSubTexture(bUIConfig::RightArrow);

			bUIImage& pauseImage = *allocator.GetElement<bUIImage>("Pause");
			pauseImage.FitParent = false;
			pauseImage.ImageSubTexture = bUI::GetConfig().GetSubTexture(bUIConfig::Pause);
		
			
			playImage.Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					m_Play = true;
				}
			});

			pauseImage.Callbacks.push_back([&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					m_Play = false;
				}
			});
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;
			m_Context->SetLength(2.0f);
		}
		void AnimationEditor::OnEvent(Event& event)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<MouseScrollEvent>(Hook(&AnimationEditor::onMouseScroll, this));
		}
		void AnimationEditor::updateLayout()
		{
			bUIAllocator& allocator = bUI::GetAllocator(GetName());
			bUIWindow& window = *allocator.GetElement<bUIWindow>("Animation Editor");
			bUITimeline& timeline = *allocator.GetElement<bUITimeline>("Timeline");
			timeline.Size.x = window.Size.x;

			if (m_Context.Raw())
			{
				timeline.CurrentTime = m_Context->GetCurrentTime();
			}
		}
		bool AnimationEditor::onMouseScroll(MouseScrollEvent& event)
		{
			if (Input::IsKeyPressed(KeyCode::KEY_LEFT_CONTROL))
			{
				bUITimeline& timeline = bUI::GetUI<bUITimeline>(GetName(), "Timeline");
				timeline.Zoom += event.GetOffsetY();
				return true;
			}
			return false;
		}
	}
}
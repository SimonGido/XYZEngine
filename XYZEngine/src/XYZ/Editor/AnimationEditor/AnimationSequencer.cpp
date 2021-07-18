#include "stdafx.h"
#include "AnimationSequencer.h"


namespace XYZ {
	namespace Editor {
		
		AnimationSequencer::AnimationSequencer()
			:
			m_FrameMin(0), 
			m_FrameMax(0),
			m_Height(200)
		{
		}
		const char* AnimationSequencer::GetItemLabel(int index) const
		{
			static char tmps[512];
			snprintf(tmps, 512, "[%02d] %s", index, m_SequencerItemTypes[m_Items[index].Type].Name.c_str());
			return tmps;
		}
		void AnimationSequencer::Get(int index, int** start, int** end, int* type, unsigned int* color)
		{
			SequenceItem& item = m_Items[index];
			if (color)
				*color = 0xFFAA8080; // same color for everyone, return color based on type
			if (start)
				*start = &item.FrameStart;
			if (end)
				*end = &item.FrameEnd;
			if (type)
				*type = item.Type;
		}

		void AnimationSequencer::DoubleClick(int index)
		{
			if (m_Items[index].Expanded)
			{
				m_Items[index].Expanded = false;
				return;
			}
			for (auto& item : m_Items)
				item.Expanded = false;
			m_Items[index].Expanded = !m_Items[index].Expanded;
		}


		void AnimationSequencer::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
		{
			draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
			
			auto& item = m_Items[index];		
			item.LineEdit.m_Max = ImVec2(float(m_FrameMax), 1.f);
			item.LineEdit.m_Min = ImVec2(float(m_FrameMin), 0.f);
			int i = 0;
			for (auto& line : item.LineEdit.GetLines())
			{
				ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
				ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
				draw_list->AddText(pta, line.Selected ? 0xFFFFFFFF : 0x80FFFFFF, line.Name.c_str());
				if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
					item.LineEdit.SetSelected(i);
				i++;
			}
		

			draw_list->PopClipRect();
		
			ImGui::SetCursorScreenPos(rc.Min);
			ImVec2 size(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);

			ImCurveEdit::Edit(item.LineEdit, size, 137 + index, &clippingRect);
		}
		void AnimationSequencer::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
		{
			draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
			auto& item = m_Items[index];
			
			item.LineEdit.m_Max = ImVec2(float(m_FrameMax), 1.f);
			item.LineEdit.m_Min = ImVec2(float(m_FrameMin), 0.f);
			for (auto& line : item.LineEdit.GetLines())
			{
				for (auto& point : line.Points)
				{
					float p = point.x;
					if (p < m_Items[index].FrameStart || p > m_Items[index].FrameEnd)
						continue;
					float r = (p - m_FrameMin) / float(m_FrameMax - m_FrameMin);
					float x = ImLerp(rc.Min.x, rc.Max.x, r);
					draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
				}
			}
			draw_list->PopClipRect();
		}
	}
}
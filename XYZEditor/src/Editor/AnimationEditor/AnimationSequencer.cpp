#include "stdafx.h"
#include "AnimationSequencer.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	namespace Editor {
		
		AnimationSequencer::AnimationSequencer(std::string name, std::vector<std::string> itemTypes)
			:
			FrameMin(0),
			FrameMax(0),
			m_SequencerItemTypes(std::move(itemTypes)),
			m_Name(std::move(name))
		{
		}
		int AnimationSequencer::GetItemTypeCount() const
		{
			return static_cast<int>(m_SequencerItemTypes.size());
		}
		const char* AnimationSequencer::GetItemTypeName(int typeIndex) const
		{
			return m_SequencerItemTypes[typeIndex].c_str();
		}
		const char* AnimationSequencer::GetItemLabel(int index) const
		{
			static char tmps[512];
			auto& item = m_Items[index];
			auto& seqItemType = m_SequencerItemTypes[item.Type];
	
			snprintf(tmps, 512, "%s", seqItemType.c_str());

			return tmps;
		}
		void AnimationSequencer::Get(int index, int** start, int** end, int* type, unsigned int* color)
		{
			const SequenceItem& item = m_Items[index];
			if (color)
				*color = 0xFFAA8080; // same color for everyone, return color based on type
			if (start)
				*start = &FrameMin; // This should be item.m_FrameStart, but we do not need it
			if (end)
				*end = &FrameMax; // This should be item.m_FrameEnd, but we do not need it
			if (type)
				*type = item.Type;
		}

		void AnimationSequencer::Copy()
		{
			m_Copy = m_Selection;
		}

		size_t AnimationSequencer::GetCustomHeight(int index)
		{
			const auto& item = m_Items[index];
			return item.Expanded ? item.LineHeight * (item.LineEdit.GetLines().size() + 1) : 0;
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
			item.LineEdit.GetMax() = ImVec2(float(FrameMax), 1.0f);
			item.LineEdit.GetMin() = ImVec2(float(FrameMin), 0.0f);
			
			int i = 0;

			for (auto& line : item.LineEdit.GetLines())
			{
				ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * item.LineHeight);
				ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * item.LineHeight);
				
				draw_list->AddText(pta, line.Selected ? 0xFFFFFFFF : 0x80FFFFFF, line.Name.c_str());
				if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
					item.LineEdit.SetSelected(i);
				i++;
			}
		

			draw_list->PopClipRect();
		
			ImGui::SetCursorScreenPos(rc.Min);
			const ImVec2 size(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);
			m_Selection.ItemIndex = index;
			ImCurveEdit::Edit(item.LineEdit, size, 137 + index, &clippingRect, &m_Selection.Points);
		}
		void AnimationSequencer::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
		{
			draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
			auto& item = m_Items[index];
			
			item.LineEdit.GetMax() = ImVec2(float(FrameMax), 1.f);
			item.LineEdit.GetMin() = ImVec2(float(FrameMin), 0.f);
			for (auto& line : item.LineEdit.GetLines())
			{
				for (auto& point : line.Points)
				{
					const float r = (point.x - FrameMin) / float(FrameMax - FrameMin);
					const float x = ImLerp(rc.Min.x, rc.Max.x, r);
					draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
				}
			}
			draw_list->PopClipRect();
		}

		void AnimationSequencer::AddItem(int type)
		{
			m_Items.push_back({ type, false, 25 });
		}

		void AnimationSequencer::AddLine(int type, const std::string& lineName, uint32_t color)
		{
			for (auto& item : m_Items)
			{
				if (item.Type == type)
				{
					item.LineEdit.AddLine(lineName, color);
					return;
				}
			}
		}
		void AnimationSequencer::AddKey(int itemIndex, int key)
		{
			auto& lineEdit = m_Items[itemIndex].LineEdit;
			size_t curveIndex = 0;
			
			if (lineEdit.GetSelectedIndex(curveIndex))
			{
				lineEdit.AddPoint(curveIndex, { static_cast<float>(key), 0.0f });
			}
		}
		void AnimationSequencer::DeleteSelectedPoints()
		{
			std::sort(m_Selection.Points.begin(), m_Selection.Points.end(), [](const ImCurveEdit::EditPoint& a, const ImCurveEdit::EditPoint& b) {
				return a.pointIndex < b.pointIndex;
			});
			for (int i = m_Selection.Points.size() - 1; i >= 0; --i)
			{
				auto& itemLine    = m_Items[m_Selection.ItemIndex].LineEdit;
				const auto& point = m_Selection.Points[i];
				itemLine.DeletePoint(point.curveIndex, point.pointIndex);
			}
			m_Selection.Points.clear();
		}
	
		bool AnimationSequencer::ItemExists(int type) const
		{
			for (const auto& item : m_Items)
			{
				if (item.Type == type)
					return true;
			}
			return false;
		}

		int AnimationSequencer::GetItemTypeIndex(std::string_view name) const
		{
			int counter = 0;
			for (const auto& itemType : m_SequencerItemTypes)
			{
				if (itemType == name)
					return counter;
				counter++;
			}
			return -1;
		}
		int AnimationSequencer::GetItemIndex(int type) const
		{
			int counter = 0;
			for (const auto& item : m_Items)
			{
				if (item.Type == type)
					return counter;
				counter++;
			}
			return -1;
		}
		int AnimationSequencer::GetItemItemType(int itemIndex) const
		{
			return m_Items[itemIndex].Type;
		}
		const SequenceLineEdit::Line* AnimationSequencer::GetSelectedLine(int itemIndex) const
		{
			if (itemIndex != -1)
			{
				return m_Items[itemIndex].LineEdit.GetSelectedLine();
			}
			for (const auto& item : m_Items)
			{
				if (const auto line = item.LineEdit.GetSelectedLine())
					return line;
			}
			return nullptr;
		}
		const SequenceLineEdit::Line& AnimationSequencer::GetLine(int itemIndex, size_t curveIndex) const
		{
			return m_Items[itemIndex].LineEdit.GetLines()[curveIndex];
		}
	}
}
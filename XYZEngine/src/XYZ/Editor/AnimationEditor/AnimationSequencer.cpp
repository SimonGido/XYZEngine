#include "stdafx.h"
#include "AnimationSequencer.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	namespace Editor {
		
		AnimationSequencer::AnimationSequencer()
			:
			m_FrameMin(0), 
			m_FrameMax(0)
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
			if (item.Entity.IsValid())
			{
				snprintf(tmps, 512, "%s:%s", item.Entity.GetComponent<SceneTagComponent>().Name.c_str(), seqItemType.c_str());
			}
			else
			{
				snprintf(tmps, 512, "%s", seqItemType.c_str());
			}
			return tmps;
		}
		void AnimationSequencer::Get(int index, int** start, int** end, int* type, unsigned int* color)
		{
			SequenceItem& item = m_Items[index];
			if (color)
				*color = 0xFFAA8080; // same color for everyone, return color based on type
			if (start)
				*start = &m_FrameMin; // This should be item.m_FrameStart, but we do not need it
			if (end)
				*end = &m_FrameMax; // This should be item.m_FrameEnd, but we do not need it
			if (type)
				*type = item.Type;
		}

		void AnimationSequencer::Copy()
		{
			m_Copy = m_Selection;
		}

		size_t AnimationSequencer::GetCustomHeight(int index)
		{
			auto& item = m_Items[index];
			return item.Expanded ? item.Height : 0;
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
			item.LineEdit.GetMax() = ImVec2(float(m_FrameMax), 1.f);
			item.LineEdit.GetMin() = ImVec2(float(m_FrameMin), 0.f);
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
			m_Selection.ItemIndex = index;
			ImCurveEdit::Edit(item.LineEdit, size, 137 + index, &clippingRect, &m_Selection.Points);
		}
		void AnimationSequencer::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
		{
			draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
			auto& item = m_Items[index];
			
			item.LineEdit.GetMax() = ImVec2(float(m_FrameMax), 1.f);
			item.LineEdit.GetMin() = ImVec2(float(m_FrameMin), 0.f);
			for (auto& line : item.LineEdit.GetLines())
			{
				for (auto& point : line.Points)
				{
					float r = (point.x - m_FrameMin) / float(m_FrameMax - m_FrameMin);
					float x = ImLerp(rc.Min.x, rc.Max.x, r);
					draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
				}
			}
			draw_list->PopClipRect();
		}
		void AnimationSequencer::AddItemType(const std::string& name)
		{
			m_SequencerItemTypes.push_back(name);
		}
		void AnimationSequencer::AddItem(int type, const SceneEntity& entity)
		{
			m_Items.push_back(SequenceItem{ type, false, entity, 100, {} });
		}
		void AnimationSequencer::AddLine(int type, const SceneEntity& entity, const std::string& lineName, uint32_t color)
		{
			for (auto& item : m_Items)
			{
				if (item.Type == type && item.Entity == entity)
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
		bool AnimationSequencer::ItemTypeExists(std::string_view name) const
		{
			for (const auto& itemType : m_SequencerItemTypes)
			{
				if (itemType == name)
					return true;
			}
			return false;
		}
		bool AnimationSequencer::ItemExists(int type, const SceneEntity& entity) const
		{
			for (const auto& item : m_Items)
			{
				if (item.Type == type && item.Entity == entity)
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
		int AnimationSequencer::GetItemIndex(int type, const SceneEntity& entity) const
		{
			int counter = 0;
			for (const auto& item : m_Items)
			{
				if (item.Type == type && item.Entity == entity)
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
				if (auto line = item.LineEdit.GetSelectedLine())
					return line;
			}
			return nullptr;
		}
	}
}
#include "stdafx.h"
#include "SequencerLineEdit.h"

namespace XYZ {
	namespace Editor {
		SequenceLineEdit::SequenceLineEdit()
            :
            m_SpacingY(0.15f),
            m_OffsetY(0.05f)
		{
		}
        void SequenceLineEdit::AddPoint(size_t curveIndex, ImVec2 value)
        {
            value.y = 1.0f - m_OffsetY - m_Lines[curveIndex].Y;
            m_Lines[curveIndex].Points.push_back(value);
            sortValues(curveIndex);
        }
        int SequenceLineEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
        {
            value.y = 1.0f - m_OffsetY - m_Lines[curveIndex].Y;
            m_Lines[curveIndex].Points[pointIndex] = ImVec2(value.x, value.y);
            sortValues(curveIndex);
            for (size_t i = 0; i < GetPointCount(curveIndex); i++)
            {
                if (m_Lines[curveIndex].Points[i].x == value.x)
                    return (int)i;
            }
            return pointIndex;
        }
        void SequenceLineEdit::AddLine(const std::string& name, uint32_t color)
        {
            size_t index = m_Lines.size();
            m_Lines.push_back({});
            auto& line = m_Lines.back();
            line.Name = name;
            line.Color = color;
            line.Y = m_SpacingY * index;
        }
   
        void SequenceLineEdit::SetSelected(size_t curveIndex)
        {
            for (size_t i = 0; i < m_Lines.size(); ++i)
                m_Lines[i].Selected = (curveIndex == i);
            
        }
        void SequenceLineEdit::Deselect()
        {
            for (size_t i = 0; i < m_Lines.size(); ++i)
                m_Lines[i].Selected = false;
        }
        void SequenceLineEdit::sortValues(size_t curveIndex)
        {
            std::sort(m_Lines[curveIndex].Points.begin(), m_Lines[curveIndex].Points.end(),
                [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
        }
	}
}
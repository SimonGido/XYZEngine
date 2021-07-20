#include "stdafx.h"
#include "SequencerLineEdit.h"

namespace XYZ {
	namespace Editor {
		SequenceLineEdit::SequenceLineEdit()
		{
		}
        void SequenceLineEdit::AddPoint(size_t curveIndex, ImVec2 value)
        {
            value.y = getLineY(curveIndex);
            m_Lines[curveIndex].Points.push_back(value);
            sortValues(curveIndex);
        }
        int SequenceLineEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
        {
            value.y = getLineY(curveIndex);          
            m_Lines[curveIndex].Points[pointIndex] = value;
            sortValues(curveIndex);
            for (size_t i = 0; i < GetPointCount(curveIndex); i++)
            {
                if (m_Lines[curveIndex].Points[i].x == value.x)
                    return (int)i;
            }
            return pointIndex;
        }
        void SequenceLineEdit::AddLine(int type, uint32_t color)
        {
            size_t index = m_Lines.size();
            m_Lines.push_back({});
            auto& line = m_Lines.back();
            line.Type = type;
            line.Color = color;
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
        const SequenceLineEdit::Line* SequenceLineEdit::GetSelectedLine() const
        {
            for (auto& line : m_Lines)
            {
                if (line.Selected)
                    return &line;
            }
            return nullptr;
        }
        bool SequenceLineEdit::GetSelectedIndex(size_t& index) const
        {
            size_t counter = 0;
            for (auto& line : m_Lines)
            {
                if (line.Selected)
                {
                    index = counter;
                    return true;
                }
                counter++;
            }
            return false;
        }
        void SequenceLineEdit::sortValues(size_t curveIndex)
        {
            std::sort(m_Lines[curveIndex].Points.begin(), m_Lines[curveIndex].Points.end(),
                [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
        }
        float SequenceLineEdit::getLineY(size_t curveIndex)
        {
            float spacing = 1.0f / (float)m_Lines.size();
            float y = spacing * curveIndex;
            return 1.0f - (spacing / 2.0f) - y;
        }
	}
}
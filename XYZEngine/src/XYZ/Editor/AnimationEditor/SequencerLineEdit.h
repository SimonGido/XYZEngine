#pragma once
#include <ImSequencer.h>
#include <ImCurveEdit.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace XYZ {
	namespace Editor {
        struct SequenceLineEdit : public ImCurveEdit::Delegate
        {
            struct Line
            {
                std::vector<ImVec2> Points;
                std::string         Name;
                uint32_t            Color;
                float               Y;
                bool                Selected;
            };

            SequenceLineEdit();
            virtual void AddPoint(size_t curveIndex, ImVec2 value);
            virtual int  EditPoint(size_t curveIndex, int pointIndex, ImVec2 value);

            virtual size_t   GetCurveCount() override { return m_Lines.size(); };
            virtual size_t   GetPointCount(size_t curveIndex) override { return m_Lines[curveIndex].Points.size(); }
            virtual uint32_t GetCurveColor(size_t curveIndex) override { return m_Lines[curveIndex].Color; }
            virtual ImVec2*  GetPoints(size_t curveIndex) override { return m_Lines[curveIndex].Points.data(); }
            virtual bool     IsVisible(size_t curveIndex) override { return true; }

       
            virtual ImVec2&                GetMax() { return m_Max; }
            virtual ImVec2&                GetMin() { return m_Min; }
            virtual unsigned int           GetBackgroundColor() { return 0; }
            virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const override { return ImCurveEdit::CurveDiscrete; }
            
            void AddLine(const std::string& name, uint32_t color = 0xFF0000FF);
            void SetSelected(size_t curveIndex);
            void Deselect();
      
            const std::vector<Line>& GetLines() const { return m_Lines; }
            const Line*              GetSelectedLine() const;
            bool                     GetSelectedIndex(size_t& index) const;


            ImVec2 m_Min;
            ImVec2 m_Max;
            float  m_SpacingY;
            float  m_OffsetY;
        private:
            void sortValues(size_t curveIndex);
           

        private:
            std::vector<Line> m_Lines;

        };
	}
}
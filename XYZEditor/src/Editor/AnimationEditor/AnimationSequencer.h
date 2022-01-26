#pragma once
#include "SequencerLineEdit.h"

#include <ImSequencer.h>
#include <ImCurveEdit.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace XYZ {
	namespace Editor {

        struct AnimationSequencer : public ImSequencer::SequenceInterface
        {
            struct Selection
            {
                ImVector<ImCurveEdit::EditPoint> Points;
                int                              ItemIndex = -1;
            };
            struct SequenceItem
            {
                int               Type;
                bool              Expanded;
                size_t            LineHeight;
                SequenceLineEdit  LineEdit;
            };

            AnimationSequencer(std::string name, std::vector<std::string> itemTypes);

            virtual int         GetFrameMin() const override { return FrameMin; }
            virtual int         GetFrameMax() const override { return FrameMax; }
            virtual int         GetItemCount() const  override { return static_cast<int>(m_Items.size()); }
            virtual int         GetItemTypeCount() const override;
            virtual const char* GetItemTypeName(int typeIndex) const override;
            virtual const char* GetItemLabel(int index) const override;
            virtual void        Get(int index, int** start, int** end, int* type, unsigned int* color) override;        
            virtual void        Del(int index) override { m_Items.erase(m_Items.begin() + index); }
            virtual void        Duplicate(int index) override { m_Items.push_back(m_Items[index]); }
            virtual void        Copy() override;

            virtual size_t      GetCustomHeight(int index) override;
            virtual void        DoubleClick(int index) override;
            virtual void        CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) override;
            virtual void        CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override;            
            
            void                AddItem(int type);
            void                AddLine(int type, const std::string& lineName, uint32_t color = 0xFF0000FF);
            void                AddKey(int itemIndex, int key);
            void                DeleteSelectedPoints();
            
            void                ClearSelection()      { m_Selection.Points.clear(); }
            void                ClearItems()          { m_Items.clear(); }
            const Selection&    GetSelection()  const { return m_Selection; }
            const Selection&    GetCopy()       const { return m_Copy; }
          
            bool                          ItemExists(int type) const;
            int                           GetItemTypeIndex(std::string_view name) const;
            int                           GetItemIndex(int type) const;
            int                           GetItemItemType(int itemIndex) const;
            const SequenceLineEdit::Line* GetSelectedLine(int itemIndex = -1) const;
            const SequenceLineEdit::Line& GetLine(int itemIndex, size_t curveIndex) const;
            const SequenceItem&           GetItem(int itemIndex) const { return m_Items[itemIndex]; }
            const std::string             GetName() const { return m_Name; }
                
        public:           
            int FrameMin;
            int FrameMax;

        private:
            std::vector<std::string>  m_SequencerItemTypes;
            std::vector<SequenceItem> m_Items;
            std::string               m_Name;

            Selection m_Selection;
            Selection m_Copy;
        };
	}
}
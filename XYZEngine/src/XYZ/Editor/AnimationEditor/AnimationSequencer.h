#pragma once
#include "SequencerLineEdit.h"
#include "XYZ/Scene/SceneEntity.h"

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
                SceneEntity       Entity;
                size_t            Height;
                SequenceLineEdit  LineEdit;
            };
            AnimationSequencer();
            virtual int         GetFrameMin() const override { return m_FrameMin; }
            virtual int         GetFrameMax() const override { return m_FrameMax; }
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
            
            void                AddItemType(const std::string& name);                  
            void                AddItem(int type, const SceneEntity& entity);
            void                AddLine(int type, const SceneEntity& entity, const std::string& lineName, uint32_t color = 0xFF0000FF);
            void                AddKey(int itemIndex, int key);
            void                DeleteSelectedPoints();
            void                ClearSelection() { m_Selection.Points.clear(); }
            const Selection&    GetSelection()  const { return m_Selection; }
            const Selection&    GetCopy()       const { return m_Copy; }
            
            bool                ItemTypeExists(std::string_view name) const;
            bool                ItemExists(int type, const SceneEntity& entity) const;

            int                           GetItemTypeIndex(std::string_view name) const;
            int                           GetItemIndex(int type, const SceneEntity& entity) const;
            int                           GetItemItemType(int itemIndex) const;
            const SequenceLineEdit::Line* GetSelectedLine(int itemIndex = -1) const;
            const SequenceLineEdit::Line& GetLine(int itemIndex, size_t curveIndex) const;
            const SequenceItem&           GetItem(int itemIndex) const { return m_Items[itemIndex]; }
        
        public:           
            int m_FrameMin;
            int m_FrameMax;

        private:
            std::vector<std::string>  m_SequencerItemTypes;
            std::vector<SequenceItem> m_Items;


            Selection m_Selection;
            Selection m_Copy;
        };
	}
}
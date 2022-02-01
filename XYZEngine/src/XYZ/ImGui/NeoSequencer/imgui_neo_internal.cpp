//
// Created by Matty on 2022-01-28.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "stdafx.h"

#include "imgui_neo_internal.h"
#include "imgui_internal.h"

namespace ImGui {
    template <typename T, typename Comp>
    static bool pushUnique(ImVector<T>& vec, const T& val, Comp comp)
    {
        for (const auto& it : vec)
        {
            if (comp(it, val))
                return false;
        }
        vec.push_back(val);
        return true;
    }
    static std::pair<ImVec2, ImVec2> getCurrentFrameLine(const ImRect& pointerBB, float timelineHeight) 
    {
        const auto center = ImVec2{ pointerBB.Min.x, pointerBB.Max.y } + ImVec2{ pointerBB.GetSize().x / 2.0f, 0 };

        return std::make_pair(center, center + ImVec2{ 0, timelineHeight });
    }


    void RenderNeoSequencerBackground(const ImVec4 &color, const ImVec2 & cursor, const ImVec2 &size, ImDrawList * drawList, float sequencerRounding) {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        const ImRect area = {cursor, cursor + size};

        drawList->AddRectFilled(area.Min, area.Max, ColorConvertFloat4ToU32(color), sequencerRounding);
    }

    void RenderNeoSequencerTopBarBackground(const ImVec4 &color, const ImVec2 &cursor, const ImVec2 &size,
                                            ImDrawList *drawList, float sequencerRounding) {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        const auto & style = GetStyle();

        const ImRect barArea = {cursor, cursor + size};

        drawList->AddRectFilled(barArea.Min, barArea.Max, ColorConvertFloat4ToU32(color), sequencerRounding);
    }

    void
    RenderNeoSequencerTopBarOverlay(float zoom, float valuesWidth,uint32_t startFrame, uint32_t endFrame, uint32_t offsetFrame, const ImVec2 &cursor, const ImVec2 &size,
                                    ImDrawList *drawList, bool drawFrameLines,
                                    bool drawFrameText) {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        const auto & style = GetStyle();

        const ImRect barArea = {cursor + ImVec2{style.FramePadding.x + valuesWidth,style.FramePadding.y}, cursor + size };

        const uint32_t viewEnd = endFrame + offsetFrame;
        const uint32_t viewStart = startFrame + offsetFrame;

        if(drawFrameLines) {
            const auto count = ((viewEnd + 1) - viewStart) / zoom;

            const auto perFrameWidth = barArea.GetSize().x / (float)count;

            for(int32_t i = 0; i < count; i++) {
                const ImVec2 p1 = {barArea.Min.x + (float)i * perFrameWidth, barArea.Max.y};

                const auto tenthFrame = ((viewStart + i) % 10 == 0);

                const auto lineHeight = tenthFrame ? barArea.GetSize().y : barArea.GetSize().y / 2.0f;

                const ImVec2 p2 = {barArea.Min.x + (float)i * perFrameWidth, barArea.Max.y - lineHeight};

                drawList->AddLine(p1,p2, IM_COL32_WHITE, 1.0f);

                if(drawFrameText && tenthFrame) {
                    char text[5];
                    itoa(viewStart + i, text, 10);
                    drawList->AddText(NULL, 0, {p1.x + 2.0f, barArea.Min.y }, IM_COL32_WHITE,text);
                }
            }
        }
    }

    void RenderNeoTimelineLabel(const char * label,const ImVec2 & cursor,const ImVec2 & size, const ImVec4& color,bool isGroup, bool isOpen, ImDrawList *drawList)
    {
        const auto& imStyle = GetStyle();

        if(!drawList) drawList = ImGui::GetWindowDrawList();
 
        auto c = cursor;

        if (isGroup) 
        {
            RenderArrow(drawList,c, IM_COL32_WHITE, isOpen ? ImGuiDir_Down : ImGuiDir_Right);
            c.x += size.y;
        }

        drawList->AddText(c,ColorConvertFloat4ToU32(color),label);
    }

    void RenderNeoTimelinesBorder(const ImVec4 &color, const ImVec2 &cursor, const ImVec2 &size, ImDrawList *drawList,
                                  float rounding, float borderSize) 
    {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(cursor,cursor + size,ColorConvertFloat4ToU32(color),rounding, 0, borderSize);
    }

    void RenderNeoTimelane(bool selected,const ImVec2 & cursor, const ImVec2& size, const ImVec4& highlightColor, ImDrawList *drawList) {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        if(selected) {
            const ImRect area = {cursor, cursor + size};
            drawList->AddRectFilled(area.Min, area.Max, ColorConvertFloat4ToU32(highlightColor));
        }
    }

    float GetPerFrameWidth(float totalSizeX, float valuesWidth, uint32_t endFrame, uint32_t startFrame, float zoom) {
        const auto& imStyle = GetStyle();

        const auto size = totalSizeX - valuesWidth - imStyle.FramePadding.x;

        const auto count = (endFrame + 1) - startFrame;

        return (size / (float)count) * zoom;
    }

    bool IsItemDoubleClicked(ImGuiMouseButton button)
    {
        return IsItemHovered() && IsMouseDoubleClicked(ImGuiMouseButton_Left);
    }

    float GetPerFrameWidth(const ImGuiNeoSequencerInternalData& context)
    {
        return GetPerFrameWidth(context.Size.x, context.ValuesWidth, context.EndFrame, context.StartFrame,
            context.Zoom);
    }

    float GetKeyframePositionX(uint32_t frame, const ImGuiNeoSequencerInternalData& context)
    {
        const auto perFrameWidth = GetPerFrameWidth(context);
        return (frame - context.OffsetFrame) * perFrameWidth;
    }

    ImVec2 GetKeyFramePosition(uint32_t frame, const ImGuiNeoSequencerInternalData& context)
    {
        const auto& imStyle = GetStyle();
        const auto timelineOffset = GetKeyframePositionX(frame, context);
        return ImVec2{ context.StartValuesCursor.x + imStyle.FramePadding.x, context.ValuesCursor.y } +
            ImVec2{ timelineOffset + context.ValuesWidth, 0 };
    }

    float GetWorkTimelineWidth(const ImGuiNeoSequencerInternalData& context)
    {
        const auto perFrameWidth = GetPerFrameWidth(context);
        return context.Size.x - context.ValuesWidth - perFrameWidth;
    }

    ImRect GetCurrentFrameBB(uint32_t frame, const ImGuiNeoSequencerInternalData& context, const ImGuiNeoSequencerStyle& style)
    {
        const auto& imStyle = GetStyle();
        const auto width = style.CurrentFramePointerSize * GetIO().FontGlobalScale;
        const auto cursor =
            context.StartCursor + ImVec2{ context.ValuesWidth + imStyle.FramePadding.x - width / 2.0f, 0 };
        const auto currentFrameCursor = cursor + ImVec2{ GetKeyframePositionX(frame, context), 0 };

        float pointerHeight = style.CurrentFramePointerSize * 2.5f;
        ImRect rect{ currentFrameCursor, currentFrameCursor + ImVec2{width, pointerHeight * GetIO().FontGlobalScale} };

        return rect;
    }

    uint32_t GetCurrentMouseFrame(const ImGuiNeoSequencerInternalData& context)
    {
        const auto& imStyle = GetStyle();
        const auto timelineXmin = context.StartCursor.x + context.ValuesWidth + imStyle.FramePadding.x;

        const ImVec2 timelineXRange = {
                timelineXmin, //min
                timelineXmin + context.Size.x - context.ValuesWidth
        };

        const auto mousePosX = GetMousePos().x;
        const auto v = mousePosX - timelineXRange.x;// Subtract min

        const auto normalized = v / GetWorkTimelineWidth(context); //Divide by width to remap to 0 - 1 range

        const auto clamped = ImClamp(normalized, 0.0f, 1.0f);

        const auto viewSize = (context.EndFrame - context.StartFrame) / context.Zoom;

        const auto frameViewVal = (float)context.StartFrame + (clamped * (float)viewSize);

        const auto finalFrame = (uint32_t)round(frameViewVal) + context.OffsetFrame;
        return finalFrame;
    }

   
    void RenderNeoSequencerCurrentFrame(const ImVec4 &color, const ImVec4 &topColor, const ImRect &pointerBB,
                                               float timelineHeight, float lineWidth, ImDrawList *drawList) {
        if(!drawList) drawList = ImGui::GetWindowDrawList();

        const auto [p0, p1] = getCurrentFrameLine(pointerBB, timelineHeight);

        drawList->AddLine(p0, p1, ColorConvertFloat4ToU32(color), lineWidth);

        { //Top pointer has custom shape, we have to create it
            const auto size = pointerBB.GetSize();
            ImVec2 pts[5];
            pts[0] = pointerBB.Min;
            pts[1] = pointerBB.Min + ImVec2{size.x, 0};
            pts[2] = pointerBB.Min + ImVec2{size.x, size.y * 0.85f};
            pts[3] = pointerBB.Min + ImVec2{size.x / 2, size.y};
            pts[4] = pointerBB.Min + ImVec2{0, size.y * 0.85f};

            drawList->AddConvexPolyFilled(pts, std::size(pts), ColorConvertFloat4ToU32(topColor));
        }
    }
    void RenderSelection(const ImVec4& color, const ImRect& rect, ImDrawList* drawList)
    {
        if (!drawList) drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(rect.Min, rect.Max, ColorConvertFloat4ToU32(color));
    }
    ImNeoSelectorState::ImNeoSelectorState()
    {
        ID = ImGuiNeoSelectorStateID_None;
        AllowedTransitions.ClearAllBits();
    }
    ImNeoSelectorStateMachine::ImNeoSelectorStateMachine()
    {
        CurrentState = &States[ImGuiNeoSelectorStateID_None];
    }
    bool ImNeoSelectorStateMachine::Transition(ImGuiNeoSelectorStateID stateID)
    {
        if (CurrentState->AllowedTransitions.TestBit(stateID))
        {
            CurrentState = &States[stateID];
            return true;
        }
        return false;
    }
    bool ImNeoSelectorStateMachine::IsInState(ImGuiNeoSelectorStateID stateID) const
    {
        return CurrentState->ID == stateID;
    }
    ImGuiNeoMultiSelector::ImGuiNeoMultiSelector()
    {
        auto& states = StateMachine.States;
        for (int i = 0; i < ImGuiNeoSelectorStateID_COUNT; ++i)
            states[i].ID = i;
        for (int i = 0; i < ImGuiNeoSelectorStateID_COUNT; ++i)
        {
            // Every state can transition to none
            states[i].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_None);
        }

        // None -> Start Select
        states[ImGuiNeoSelectorStateID_None].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_StartSelect);
        // Start Select -> Edit Ready // Single key selection
        states[ImGuiNeoSelectorStateID_StartSelect].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_EditReady);
        // Start Multi Select -> Multi Selecting
        states[ImGuiNeoSelectorStateID_StartSelect].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_MultiSelecting);
        // Multi Selecting -> Edit Ready
        states[ImGuiNeoSelectorStateID_MultiSelecting].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_EditReady);
        // Edit Ready -> Start Editing
        states[ImGuiNeoSelectorStateID_EditReady].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_StartEditing);
        // Start Editing -> Editing Selection
        states[ImGuiNeoSelectorStateID_StartEditing].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_EditingSelection);   
        // Editing Selection -> Edit Ready
        states[ImGuiNeoSelectorStateID_EditingSelection].AllowedTransitions.SetBit(ImGuiNeoSelectorStateID_EditReady);
    }
    bool ImGuiNeoMultiSelector::SetSingleKeySelection(uint32_t keyindex, ImGuiID timelineID, const ImRect& bb)
    {
        if (StateMachine.Transition(ImGuiNeoSelectorStateID_EditReady))
        {
            KeyFrameSelection.clear();
            KeyFrameSelection[timelineID].push_back(keyindex);
             // Context selection rect is only single point
            SelectedRect = bb;
            
            return true;
        }
        return false;
    }
    bool ImGuiNeoMultiSelector::IsKeyFrameSelected(uint32_t keyIndex, ImGuiID timelineID) const
    {
        const auto currentLineSelection = KeyFrameSelection.find(timelineID);
        if (currentLineSelection == KeyFrameSelection.end())
            return false;

        const ImVector<uint32_t>& keys = currentLineSelection->second;
        for (const auto key : keys)
        {
            if (key == keyIndex)
                return true;
        }

        return false;
    }
    bool ImGuiNeoMultiSelector::HandleKeyFrameEdit(uint32_t* keyframes, uint32_t keyframeCount, ImGuiID timelineID)
    {
        if (!ImGui::IsSequencerHovered()
         || !StateMachine.IsInState(ImGuiNeoSelectorStateID_EditingSelection))
            return false;

        bool handled = false;
        const int32_t diffFrame = static_cast<int32_t>(CurrentEditFrame) - static_cast<int32_t>(LastEditFrame);

        const auto& currentLineSelection = KeyFrameSelection[timelineID];
        for (const auto keyFrameIndex : currentLineSelection)
        {
            for (uint32_t i = 0; i < keyframeCount; ++i)
            {
                if (keyFrameIndex == i)
                {
                    if (IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
                    {
                        keyframes[keyFrameIndex] += diffFrame;
                    }
                    handled = true;
                    break;
                }
            }
        }
        return handled;
    }
    bool ImGuiNeoMultiSelector::AttemptToAddToSelection(uint32_t keyIndex, const ImRect& keyBB, ImGuiID timelineID)
    {
        if (StateMachine.IsInState(ImGuiNeoSelectorStateID_MultiSelecting))
        {
            bool added = false;
            bool removed = false;
            auto& currentLineSelection = KeyFrameSelection[timelineID];
            if (MultiSelectingRect.Overlaps(keyBB))
            {
                added = pushUnique(
                    currentLineSelection, keyIndex,
                    [](const uint32_t a, const uint32_t b) {
                    return a == b;
                });
            }
            else // If it is no longer inside selection rect, remove it
            {
                removed = currentLineSelection.find_erase(keyIndex);
            }

            // This is quite inefficient / but comparing integers is no big deal
            if (currentLineSelection.contains(keyIndex))
            {
                SelectedRect.Min.x = std::min(keyBB.Min.x, SelectedRect.Min.x);
                SelectedRect.Min.y = std::min(keyBB.Min.y, SelectedRect.Min.y);

                SelectedRect.Max.x = std::max(keyBB.Max.x, SelectedRect.Max.x);
                SelectedRect.Max.y = std::max(keyBB.Max.y, SelectedRect.Max.y);
            }
            if (removed)
            {
                const float bigNumber = 999999.9f;
                SelectedRect.Min = { bigNumber, bigNumber };
                SelectedRect.Max = { -bigNumber, -bigNumber };
            }
            return added;
        }
        return false;
    }
    bool ImGuiNeoMultiSelector::HandleMultiSelection(const ImGuiNeoSequencerInternalData& context)
    {
        if (IsSequencerHovered())
        {
            if (IsSequencerClicked(ImGuiMouseButton_Left)
                && !context.HoldingCurrentFrame)
            {
                if (StateMachine.Transition(ImGuiNeoSelectorStateID_StartSelect))
                {
                    MultiSelectingRect.Min = ImGui::GetMousePos();
                    MultiSelectingRect.Max = MultiSelectingRect.Min;

                    const float bigNumber = 999999.9f;
                    SelectedRect.Min = ImVec2(bigNumber, bigNumber);
                    SelectedRect.Max = ImVec2(-bigNumber, -bigNumber);
                    return true;
                }
            }

            const float dragTreshold = 0.01f;

            if (IsMouseDragging(ImGuiMouseButton_Left, dragTreshold))
            {
                StateMachine.Transition(ImGuiNeoSelectorStateID_MultiSelecting);
                if (StateMachine.IsInState(ImGuiNeoSelectorStateID_MultiSelecting))
                {
                    MultiSelectingRect.Max = ImGui::GetMousePos();
                    return true;
                }
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            // If selection is not empty, we are ready to edit
            if (!KeyFrameSelection.empty())
                return StateMachine.Transition(ImGuiNeoSelectorStateID_EditReady);
            else
                StateMachine.Transition(ImGuiNeoSelectorStateID_None);
        }
        return false;
    }

    bool ImGuiNeoMultiSelector::HandleEditSelection(const ImGuiNeoSequencerInternalData& context)
    {
        if (IsSequencerHovered())
        {
            const float dragTreshold = 0.0f;
            if (IsSequencerClicked(ImGuiMouseButton_Left))
            {
                // We are ready to start editing
                if (StateMachine.Transition(ImGuiNeoSelectorStateID_StartEditing))
                {              
                    // Check if we clicked on selected rect
                    if (SelectedRect.Contains(GetMousePos()))
                    {
                        CurrentEditFrame = GetCurrentMouseFrame(context);
                        LastEditFrame = CurrentEditFrame;
                        return true;
                    }
                    // We did not click on selected rect, so perform transition to none
                    else
                    {
                        KeyFrameSelection.clear();
                        StateMachine.Transition(ImGuiNeoSelectorStateID_None);
                    }
                }
            }
            else if (IsMouseDragging(ImGuiMouseButton_Left, dragTreshold))
            {
                // Try to transit to editing selection state;
                StateMachine.Transition(ImGuiNeoSelectorStateID_EditingSelection);
               
                // We are in editing state and dragging update frames and selected rectangle
                if (StateMachine.IsInState(ImGuiNeoSelectorStateID_EditingSelection))
                {
                    LastEditFrame = CurrentEditFrame;
                    CurrentEditFrame = GetCurrentMouseFrame(context);
                    const int32_t diff = CurrentEditFrame - LastEditFrame;
                    const float frameWidth = GetPerFrameWidth(context);
                    SelectedRect.Min.x += diff * frameWidth;
                    SelectedRect.Max.x += diff * frameWidth;
                    return true;
                }
            }
            // Sequencer is hovered but mouse is released so transit to edit ready
            else if (IsMouseReleased(ImGuiMouseButton_Left))
            {
                StateMachine.Transition(ImGuiNeoSelectorStateID_EditReady);
            }
        }
        return false;
    }
    bool ImGuiNeoMultiSelector::IsMultiSelecting() const
    {
        return StateMachine.IsInState(ImGuiNeoSelectorStateID_MultiSelecting);
    }
    bool ImGuiNeoMultiSelector::IsEditingSelection() const
    {
        return StateMachine.IsInState(ImGuiNeoSelectorStateID_EditingSelection);
    }
}

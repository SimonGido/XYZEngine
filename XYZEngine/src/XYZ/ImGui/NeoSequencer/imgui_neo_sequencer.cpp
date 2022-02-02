//
// Created by Matty on 2022-01-28.
//
#define IMGUI_DEFINE_MATH_OPERATORS
#include "stdafx.h"

#include "imgui_neo_sequencer.h"
#include "imgui_internal.h"
#include "imgui_neo_internal.h"

#include <stack>
#include <unordered_set>


namespace ImGui {

    #define BIT_SET(flags, flag) (flags & flag)
  
    static ImGuiNeoSequencerStyle     style; // NOLINT(cert-err58-cpp)
    static ImGuiNeoSequencerFrameData frameData;


    // Data of all sequencers, this is main c++ part and I should create C alternative or use imgui ImVector or something
    static std::unordered_map<ImGuiID, ImGuiNeoSequencerInternalData> sequencerData;


    ///////////// STATIC HELPERS ///////////////////////
    

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

    static ImVec2 getRealSize(const ImVec2 size, const ImGuiNeoSequencerInternalData& context)
    {
        const auto area = ImGui::GetContentRegionAvail();
        auto realSize = ImFloor(size);
        if (realSize.x <= 0.0f)
            realSize.x = ImMax(4.0f, area.x);
        if (realSize.y <= 0.0f)
            realSize.y = ImMax(4.0f, context.FilledHeight);

        return realSize;
    }

    static bool dragKeyFrame(uint32_t* frame, const ImGuiNeoSequencerInternalData& context)
    {
        if (IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
        {
            *frame = GetCurrentMouseFrame(context);
            return true;
        }
        return false;
    }

  
    static void processCurrentFrame(uint32_t* frame, ImGuiNeoSequencerInternalData& context)
    {
        auto pointerRect = GetCurrentFrameBB(*frame, context, style);
        pointerRect.Min -= ImVec2{ 2.0f, 2.0f };
        pointerRect.Max += ImVec2{ 2.0f, 2.0f };


        if (!ItemAdd(pointerRect, 0))
            return;

        context.CurrentFrameColor = GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_FramePointer);

        if (IsItemHovered())
        {
            context.CurrentFrameColor = GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_FramePointerHovered);
        }

        if (context.HoldingCurrentFrame)
        {
            if (dragKeyFrame(frame, context))
                context.CurrentFrameColor = GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_FramePointerPressed);

            if (!IsMouseDown(ImGuiMouseButton_Left))
            {
                context.HoldingCurrentFrame = false;
                context.CurrentFrameColor = GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_FramePointer);
            }
        }

        if (IsItemClicked() && !context.HoldingCurrentFrame)
        {
            context.HoldingCurrentFrame = true;
            context.CurrentFrameColor = GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_FramePointerPressed);
        }

        context.CurrentFrame = *frame;
    }

    static void finishPreviousTimeline(ImGuiNeoSequencerInternalData& context)
    {
        context.ValuesCursor = { context.StartCursor.x, context.ValuesCursor.y };
        frameData.CurrentTimelineHeight = 0.0f;
    }

    static bool processKeyframe(uint32_t *frame, uint32_t index) 
    {
        auto &context = sequencerData[frameData.CurrentSequencer];

        const auto pos = GetKeyFramePosition(*frame, context);

        const auto bbPos = pos - ImVec2{frameData.CurrentTimelineHeight / 2, 0};

        const ImRect bb = {bbPos, bbPos + ImVec2{frameData.CurrentTimelineHeight, frameData.CurrentTimelineHeight}};

        if (!ItemAdd(bb, 0))
            return false;

        const auto drawList = ImGui::GetWindowDrawList();

        uint32_t color = 0;
        
        // Single click selection
        if (IsItemClicked(ImGuiMouseButton_Left))
        {
            context.Selector.SetSingleKeySelection(*frame, index, frameData.CurrentTimeline, bb);
        }
        else
        {
            context.Selector.AttemptToAddToSelection(*frame, index, bb, frameData.CurrentTimeline);
        }

        if (IsItemHovered())
        {
            color = ColorConvertFloat4ToU32(style.Colors[ImGuiNeoSequencerCol_KeyframeHovered]);
        }
        else if (context.Selector.IsKeyFrameSelected(index, frameData.CurrentTimeline))
        {
            color = ColorConvertFloat4ToU32(style.Colors[ImGuiNeoSequencerCol_KeyframeSelected]);
        }
        else
        {
            color = ColorConvertFloat4ToU32(style.Colors[ImGuiNeoSequencerCol_Keyframe]);
        }
        drawList->AddCircleFilled(pos + ImVec2{0, frameData.CurrentTimelineHeight / 2.f}, 
                                  frameData.CurrentTimelineHeight / 3.0f,
                                  color, 4);

        return true;
    }

    static void renderCurrentFrame(const ImGuiNeoSequencerInternalData &context) 
    {
        const auto bb = GetCurrentFrameBB(context.CurrentFrame, context, style);

        const auto drawList = ImGui::GetWindowDrawList();

        RenderNeoSequencerCurrentFrame(
                GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_TimelineBorder),
                context.CurrentFrameColor,
                bb,
                context.Size.y - context.TopBarSize.y,
                style.CurrentFrameLineWidth,
                drawList
        );
    }
    static bool groupBehaviour(const ImGuiID id, bool* open, const ImVec2 labelSize)
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        ImGuiWindow* window = GetCurrentWindow();

        const bool closable = open != nullptr;

        auto drawList = ImGui::GetWindowDrawList();
        const float arrowWidth = drawList->_Data->FontSize;
        const ImVec2 arrowSize = { arrowWidth, arrowWidth };
        const ImRect arrowBB = {
            context.ValuesCursor,
            context.ValuesCursor + arrowSize
        };
        const ImVec2 groupBBMin = { context.ValuesCursor + ImVec2{arrowSize.x, 0.0f} };
        const ImRect groupBB = {
               groupBBMin,
               groupBBMin + labelSize
        };


        const ImGuiID arrowID = window->GetID(frameData.TimelineIndex * 1000);
        const auto addArrowRes = ItemAdd(arrowBB, arrowID);
        if (addArrowRes)
        {
            if (IsItemClicked() && closable)
                (*open) = !(*open);
        }

        const auto addGroupRes = ItemAdd(groupBB, id);
        if (addGroupRes)
        {
            if (IsItemClicked())
            {
                context.SelectedTimeline = context.SelectedTimeline == id ? 0 : id;
            }
        }
        const float width = groupBB.Max.x - arrowBB.Min.x;
        context.ValuesWidth = std::max(context.ValuesWidth, width); // Make left panel wide enough
        return addGroupRes && addArrowRes;
    }

    static bool timelineBehaviour(const ImGuiID id, const ImVec2 labelSize)
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        ImGuiWindow* window = GetCurrentWindow();

        const ImRect groupBB = {
               context.ValuesCursor,
               context.ValuesCursor + labelSize
        };

        const auto addGroupRes = ItemAdd(groupBB, id);
        if (addGroupRes)
        {
            if (IsItemClicked())
            {
                context.SelectedTimeline = context.SelectedTimeline == id ? 0 : id;
            }
        }
        const float width = groupBB.Max.x - groupBB.Min.x;
        context.ValuesWidth = std::max(context.ValuesWidth, width); // Make left panel wide enough

        return addGroupRes;
    }
    
    static void handleSequencerInput(ImGuiID id, ImGuiNeoSequencerInternalData& context, float *zoom, ImGuiNeoSequencerFlags flags)
    {
        if (IsItemHovered())
        {
            frameData.HoveredSequencer = id;
            if (BIT_SET(flags, ImGuiNeoSequencerFlags_MouseScroll))
            {
                ImGui::SetItemUsingMouseWheel();
                *zoom += ImGui::GetIO().MouseWheel * style.MouseScrollSpeed;
            }
        }
        if (IsItemClicked(ImGuiMouseButton_Left))
        {
            frameData.ClickedSequencer[ImGuiMouseButton_Left] = id;
        }
        else if (IsItemClicked(ImGuiMouseButton_Right))
        {
            frameData.ClickedSequencer[ImGuiMouseButton_Right] = id;
        }
    }

    ////////////////////////////////////


    const ImVec4 &GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol idx) 
    {
        return GetNeoSequencerStyle().Colors[idx];
    }

    ImGuiNeoSequencerStyle &GetNeoSequencerStyle() 
    {
        return style;
    }

    ImVector<uint32_t>& GetCurrentTimelineSelection()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.Selector.KeyFrameSelection[frameData.CurrentTimeline];
    }

    bool IsMultiselecting()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.Selector.StateMachine.IsInState(ImGuiNeoSelectorStateID_MultiSelecting);
    }

    bool IsEditingSelection()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.Selector.StateMachine.IsInState(ImGuiNeoSelectorStateID_EditingSelection);
    }

    IMGUI_API bool IsCurrentTimelineSelected()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.SelectedTimeline == frameData.CurrentTimeline;
    }

    IMGUI_API bool KeyFramesEdited()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.KeyFramesEdited;
    }

    IMGUI_API ImGuiID GetCurrentTimelineID()
    {
        return frameData.CurrentTimeline;
    }

    IMGUI_API ImGuiID GetSelectedTimelineID()
    {
        auto& context = sequencerData[frameData.CurrentSequencer];
        return context.SelectedTimeline;
    }

    bool
        BeginNeoSequencer(const char* idin, uint32_t* frame, uint32_t* startFrame, uint32_t* endFrame, uint32_t* offsetFrame, float* zoom,
            const ImVec2& size, ImGuiNeoSequencerFlags flags)
    {
        IM_ASSERT(!frameData.InSequencer && "Called when while in other NeoSequencer, that won't work, call End!");

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        const auto& imStyle = GetStyle();
        auto& neoStyle = GetNeoSequencerStyle();

        if (frameData.InSequencer)
            return false;

        if (window->SkipItems)
            return false;


        PushID(idin);
        const auto id = window->IDStack[window->IDStack.size() - 1];
        bool first = sequencerData.count(id) == 0;
        auto& context = sequencerData[id];

        const auto cursor = GetCursorScreenPos();
        const auto cursorBasePos = GetCursorScreenPos();
        const ImRect clip = { 
            cursorBasePos, 
            cursorBasePos + context.Size
        };

        ItemAdd(clip, id);
        ItemHoverable(clip, id);
        
        handleSequencerInput(id, context, zoom, flags);

        frameData.CurrentSequencer = id;
        frameData.InSequencer = true;

  
        context.StartCursor = cursor;
        context.StartFrame = *startFrame;
        context.EndFrame = *endFrame;
        context.OffsetFrame = *offsetFrame;
        context.Zoom = *zoom;
        context.Size = getRealSize(size, context);

        const auto drawList = GetWindowDrawList();
        RenderNeoSequencerBackground(GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_Bg), context.StartCursor,
                                     context.Size,
                                     drawList, style.SequencerRounding);
        
        RenderNeoSequencerTopBarBackground(GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_TopBarBg),
                                           context.StartCursor, context.TopBarSize,
                                           drawList, style.SequencerRounding);

        RenderNeoSequencerTopBarOverlay(context.Zoom, context.ValuesWidth, context.StartFrame, context.EndFrame,
                                        context.OffsetFrame,
                                        context.StartCursor, context.TopBarSize, drawList,
                                        style.TopBarShowFrameLines, style.TopBarShowFrameTexts);
        
        if (context.Selector.IsMultiSelecting())
            RenderSelection(style.Colors[ImGuiNeoSequencerCol_MultiSelectColor], context.Selector.MultiSelectingRect);
        
        if (context.Selector.IsEditingSelection() || context.Selector.IsEditReady())
            RenderSelection(style.Colors[ImGuiNeoSequencerCol_MultiSelectColor], context.Selector.SelectedRect);

        context.TopBarSize = ImVec2(context.Size.x, style.TopBarHeight);

        if (context.TopBarSize.y <= 0.0f)
            context.TopBarSize.y = CalcTextSize("100").y + imStyle.FramePadding.y * 2.0f;

        if (context.Size.y < context.FilledHeight)
            context.Size.y = context.FilledHeight;

        context.FilledHeight = context.TopBarSize.y + style.TopBarSpacing;

        context.StartValuesCursor = context.StartCursor + ImVec2{0, context.TopBarSize.y + style.TopBarSpacing};
        context.ValuesCursor = context.StartValuesCursor;

        processCurrentFrame(frame, context);
        context.KeyFramesEdited = context.Selector.HandleEditSelection(context);
        context.Selector.HandleMultiSelection(context);
        return true;
    }

    void EndNeoSequencer() 
    {
        IM_ASSERT(frameData.InSequencer && "Called end sequencer when BegframeData.InSequencer didnt return true or wasn't called at all!");
        IM_ASSERT(sequencerData.count(frameData.CurrentSequencer) != 0 && "Ended sequencer has no context!");

        auto &context = sequencerData[frameData.CurrentSequencer];
        auto &imStyle = GetStyle();
        renderCurrentFrame(context);

        frameData.InSequencer = false;

        const ImVec2 min = {0, 0};
        context.Size.y = context.FilledHeight;
        const auto max = context.Size;


        ItemSize({min, max});       
        PopID();
    }

    IMGUI_API bool BeginNeoGroup(const char *label, bool *open)
    {
        return BeginNeoTimeline(label, nullptr, 0, nullptr, open, ImGuiNeoTimelineFlags_Group);
    }

    IMGUI_API void EndNeoGroup() 
    {
        return EndNeoTimeLine();
    }

   

    bool BeginNeoTimeline(const char *label, ImNeoKeyFrame *keyframes, uint32_t keyframeCount, ImNeoKeyChangeFn func, bool *open, ImGuiNeoTimelineFlags flags) {
        IM_ASSERT(frameData.InSequencer && "Not in active sequencer!");
        IM_ASSERT(IsSorted(keyframes, keyframeCount) && "Key frames are not sorted"); // This assert might cause performance issues in debug mode

        const bool closable = open != nullptr;
        auto &context = sequencerData[frameData.CurrentSequencer];
        const auto &imStyle = GetStyle();
        ImGuiWindow *window = GetCurrentWindow();
        const ImGuiID id = window->GetID(label);
        
        frameData.TimelineIndex++;
        frameData.CurrentTimeline = id;

        auto labelSize = CalcTextSize(label);

        labelSize.y += imStyle.FramePadding.y * 2 + style.ItemSpacing.y * 2;
        labelSize.x += imStyle.FramePadding.x * 2 + style.ItemSpacing.x * 2 +
                       (float) frameData.CurrentTimelineDepth * style.DepthItemSpacing;


        bool isGroup = flags & ImGuiNeoTimelineFlags_Group && closable;
        bool addRes = false;
        if (isGroup)
        {
            labelSize.x += imStyle.ItemSpacing.x + GetFontSize();
            addRes = groupBehaviour(id, open, labelSize);
        }
        else
        {
            addRes = timelineBehaviour(id, labelSize);
        }

        if (frameData.CurrentTimelineDepth > 0) {
            context.ValuesCursor = {context.StartCursor.x, context.ValuesCursor.y};
        }

        frameData.CurrentTimelineHeight = labelSize.y;
        context.FilledHeight += frameData.CurrentTimelineHeight;

        // Rendering
        if (addRes) 
        {
            RenderNeoTimelane(id == context.SelectedTimeline,
                              context.ValuesCursor + ImVec2{context.ValuesWidth, 0},
                              ImVec2{context.Size.x - context.ValuesWidth, frameData.CurrentTimelineHeight},
                              GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol_SelectedTimeline));

            ImVec4 color = GetStyleColorVec4(ImGuiCol_Text);
            if (IsItemHovered())  color.w *= 0.7f;

            RenderNeoTimelineLabel(label,
                                   context.ValuesCursor + imStyle.FramePadding +
                                   ImVec2{(float)frameData.CurrentTimelineDepth * style.DepthItemSpacing, 0},
                                   labelSize,
                                   color,
                                   isGroup,
                                   isGroup && (*open));        
        
            for (uint32_t i = 0; i < keyframeCount; i++) 
            {
                bool keyframeRes = processKeyframe(&keyframes[i].Frame, i); // Draw keyframe and add to selection
            }     
        }
        context.Selector.HandleKeyFrameEdit(keyframes, keyframeCount, frameData.CurrentTimeline, func);


        context.ValuesCursor.x += imStyle.FramePadding.x + (float) frameData.CurrentTimelineDepth * style.DepthItemSpacing;
        context.ValuesCursor.y += frameData.CurrentTimelineHeight;

        const auto result = !closable || (*open);

        if (result) 
        {
            frameData.CurrentTimelineDepth++;
        } 
        else 
        {
            finishPreviousTimeline(context);
        }
        return result;
    }

    void EndNeoTimeLine() 
    {
        auto &context = sequencerData[frameData.CurrentSequencer];
        finishPreviousTimeline(context);
        frameData.CurrentTimelineDepth--;
    }


    bool NeoBeginCreateKeyframe(uint32_t *frame) {
        return false;
    }

    bool NeoZoomControl(const char* label, float* zoom, uint32_t* offsetFrame, float speed, bool overlay)
    {
        auto& imStyle = GetStyle();
        const auto& context = sequencerData[frameData.CurrentSequencer];

        const ImVec2 oldPos = ImGui::GetCursorPos();
        if (overlay)
        {
            const ImVec2 winSize = ImGui::GetWindowSize();
            const ImVec2 label_size = CalcTextSize(label, NULL, true);
            const ImVec2 controlSize = CalcItemSize(ImVec2{ 0.0f, 0.0f }, label_size.x + imStyle.FramePadding.x * 2.0f, label_size.y + imStyle.FramePadding.y * 2.0f);
            const float offset = 2.0f;
            const ImVec2 newCursorPos = ImVec2{ 2.0f * imStyle.FramePadding.x, winSize.y - 2.0f * imStyle.FramePadding.y - controlSize.y - offset };
            ImGui::SetCursorPos(newCursorPos);
        }

        bool result = false;
        float viewStart = static_cast<float>(*offsetFrame);
        const float viewWidth = context.EndFrame - context.StartFrame;
        const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * GImGui->Font->Scale * 2.0f;
        float sliderWidth = context.Size.x - 2 * lineHeight;

        ImGui::PushItemWidth(sliderWidth);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, context.Size.x / context.Zoom);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));


        ImGui::Button("<", ImVec2(lineHeight, 0.0f));
        if (ImGui::IsItemActive())
        {
            *zoom -= speed;
            result = true;
        }
        ImGui::SameLine();
       
      
        if (ImGui::SliderFloat(label, &viewStart, 0.0f, context.EndFrame, ""))
        {
            *offsetFrame = static_cast<uint32_t>(viewStart);
            result = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Button(">", ImVec2(lineHeight, 0.0f));
        if (ImGui::IsItemActive())
        {
            *zoom += speed;
            result = true;
        }
        ImGui::PopStyleVar(2);
          
        if (overlay)
            ImGui::SetCursorPos(oldPos);

        return result;
    }


    void PushNeoSequencerStyleColor(ImGuiNeoSequencerCol idx, ImU32 col) 
    {
        ImGuiColorMod backup;
        backup.Col = idx;
        backup.BackupValue = style.Colors[idx];
        frameData.SequencerColorStack.push_back(backup);
        style.Colors[idx] = ColorConvertU32ToFloat4(col);
    }

    void PushNeoSequencerStyleColor(ImGuiNeoSequencerCol idx, const ImVec4 &col) 
    {
        ImGuiColorMod backup;
        backup.Col = idx;
        backup.BackupValue = style.Colors[idx];
        frameData.SequencerColorStack.push_back(backup);
        style.Colors[idx] = col;
    }

    void PopNeoSequencerStyleColor(int count) 
    {
        while (count > 0)
        {
            ImGuiColorMod& backup = frameData.SequencerColorStack.back();
            style.Colors[backup.Col] = backup.BackupValue;
            frameData.SequencerColorStack.pop_back();
            count--;
        }
    }

    bool IsSequencerHovered()
    {
        return frameData.CurrentSequencer == frameData.HoveredSequencer;
    }
    bool IsSequencerClicked(ImGuiMouseButton button)
    {
        return frameData.CurrentSequencer == frameData.ClickedSequencer[button];
    }
    void BeginNeo()
    {
        frameData.HoveredSequencer = 0;
        for (auto& id : frameData.ClickedSequencer)
            id = 0;
    }
    void EndNeo()
    {
        
    }
}

ImGuiNeoSequencerStyle::ImGuiNeoSequencerStyle() 
{
    Colors[ImGuiNeoSequencerCol_Bg] = ImVec4{0.31f, 0.31f, 0.31f, 1.00f};
    Colors[ImGuiNeoSequencerCol_TopBarBg] = ImVec4{0.22f, 0.22f, 0.22f, 0.84f};
    Colors[ImGuiNeoSequencerCol_SelectedTimeline] = ImVec4{0.98f, 0.706f, 0.322f, 0.88f};
    Colors[ImGuiNeoSequencerCol_TimelinesBg] = Colors[ImGuiNeoSequencerCol_TopBarBg];
    Colors[ImGuiNeoSequencerCol_TimelineBorder] = Colors[ImGuiNeoSequencerCol_Bg] * ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

    Colors[ImGuiNeoSequencerCol_FramePointer] = ImVec4{0.98f, 0.24f, 0.24f, 0.50f};
    Colors[ImGuiNeoSequencerCol_FramePointerHovered] = ImVec4{0.98f, 0.15f, 0.15f, 1.00f};
    Colors[ImGuiNeoSequencerCol_FramePointerPressed] = ImVec4{0.98f, 0.08f, 0.08f, 1.00f};

    Colors[ImGuiNeoSequencerCol_Keyframe] = ImVec4{0.49f, 0.49f, 0.49f, 1.00f};
    Colors[ImGuiNeoSequencerCol_KeyframeHovered] = ImVec4{0.98f, 0.39f, 0.36f, 1.00f};
    Colors[ImGuiNeoSequencerCol_KeyframeSelected] = ImVec4{0.4f, 0.8f, 0.9f, 1.00f};
    Colors[ImGuiNeoSequencerCol_MultiSelectColor] = ImVec4{ 0.1f, 0.3f, 0.9f, 0.3f };
}

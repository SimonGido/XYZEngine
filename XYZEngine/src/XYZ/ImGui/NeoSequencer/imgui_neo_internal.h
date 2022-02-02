//
// Created by Matty on 2022-01-28.
//

#ifndef IMGUI_NEO_INTERNAL_H
#define IMGUI_NEO_INTERNAL_H

#include "imgui.h"
#include "imgui_internal.h"

#include "imgui_neo_sequencer.h"

namespace ImGui {
   
    struct ImGuiNeoSequencerFrameData
    {
        //Global context stuff
        bool InSequencer = false;

        // Height of timeline right now
        float CurrentTimelineHeight = 0.0f;

        // Current active sequencer
        ImGuiID CurrentSequencer = 0;

        ImGuiID ClickedSequencer[ImGuiMouseButton_COUNT];
        ImGuiID HoveredSequencer = 0;

        ImGuiID CurrentTimeline = 0; // Current active timeline

        // Current timeline depth, used for offset of label
        uint32_t CurrentTimelineDepth = 0;

        // It is basically timeline counter of current sequencer
        uint32_t TimelineIndex = 0;

        ImVector<ImGuiColorMod> SequencerColorStack;
    };

    enum ImNeoSelectorStateID_
    {
        ImGuiNeoSelectorStateID_None,

        ImGuiNeoSelectorStateID_StartSelect,
        ImGuiNeoSelectorStateID_MultiSelecting,

        ImGuiNeoSelectorStateID_EditReady,
        ImGuiNeoSelectorStateID_StartEditing,
        ImGuiNeoSelectorStateID_EditingSelection,
        ImGuiNeoSelectorStateID_COUNT
    };

   

    
    using ImSelectionMap = std::unordered_map<ImGuiID, ImVector<uint32_t>>;
    typedef int ImGuiNeoSelectorStateID;

    struct ImNeoSelectorState
    {
        ImNeoSelectorState();

        ImBitArray<ImGuiNeoSelectorStateID_COUNT> AllowedTransitions;
        ImGuiNeoSelectorStateID                   ID;
    };
    struct ImNeoSelectorStateMachine
    {
        ImNeoSelectorStateMachine();

        bool Transition(ImGuiNeoSelectorStateID stateID);
        bool IsInState(ImGuiNeoSelectorStateID stateID) const;

        ImNeoSelectorState  States[ImGuiNeoSelectorStateID_COUNT];
        ImNeoSelectorState* CurrentState = nullptr;
    };


    struct ImGuiNeoSequencerInternalData;
    struct ImGuiNeoMultiSelector
    {
        ImGuiNeoMultiSelector();

        ImNeoSelectorStateMachine StateMachine;             
        ImSelectionMap            KeyFrameSelection;        // Frames in multiselection
        ImRect                    MultiSelectingRect;       // Selecting rectangle
        ImRect                    SelectedRect;             // Rectangle created from selected keyframes
        uint32_t                  SelectedRectMinFrame;     // Min frame in selected rect
        uint32_t                  SelectedRectMaxFrame;     // Max frame in selected rect
        uint32_t                  LastEditFrame = 0;        // Frame on which editing was last frame
        uint32_t                  CurrentEditFrame = 0;     // Current frame editing
        int32_t                   DiffFrame = 0;            // Diff current and last frame ( used for moving keys )
        bool                      DiffOutOfBounds = false;  // == SelectedRectMinFrame + DiffFrame < 0
  
        bool SetSingleKeySelection(uint32_t keyframe, uint32_t keyindex, ImGuiID timelineID, const ImRect& bb);

        bool IsKeyFrameSelected(uint32_t keyIndex, ImGuiID timelineID) const;

        bool HandleKeyFrameEdit(ImNeoKeyFrame* keyframes, uint32_t keyframeCount, ImGuiID timelineID, ImNeoKeyChangeFn func);

        bool AttemptToAddToSelection(uint32_t keyframe, uint32_t keyIndex, const ImRect& keyBB, ImGuiID timelineID);

        bool HandleMultiSelection(const ImGuiNeoSequencerInternalData& context);

        bool HandleEditSelection(const ImGuiNeoSequencerInternalData& context);
        
        bool IsMultiSelecting() const;
        bool IsEditingSelection() const;
        bool IsEditReady() const;
        bool IsSelectionEmpty() const;

    private:
        // Store default values of Selected Rect/MinFrame/MaxFrame
        void resetSelected();

        // Check if min/max values are stored in Selected Rect/MinFrame/MaxFrame
        void updateSelected(const ImRect& keyBB, uint32_t keyframe);

        // Add diff to Selected Rect/MinFrame/MaxFrame
        void updateSelectedEdit(const int32_t diff, float frameWidth);

        // Updates LastEditFrame, CurrentEditDrame and DiffOutOfBounds
        void updateDiffFrame(const ImGuiNeoSequencerInternalData& context);
    };

    struct ImGuiNeoSequencerInternalData
    {
        ImVec2 StartCursor = { 0, 0 }; // Cursor on top, below zoom slider
        ImVec2 StartValuesCursor = { 0, 0 }; // Cursor on top of values
        ImVec2 ValuesCursor = { 0, 0 }; // Current cursor position, used for values drawing

        ImVec2 Size = { 0, 0 }; // Size of whole sequencer
        ImVec2 TopBarSize = { 0, 0 }; // Size of top bar without zoom

        uint32_t StartFrame = 0;
        uint32_t EndFrame = 0;
        uint32_t OffsetFrame = 0; // Offset from start

        float    ValuesWidth = 32.0f; // Width of biggest label in timeline, used for offset of timeline

        float    FilledHeight = 0.0f; // Height of whole sequencer

        float    Zoom = 1.0f;

        ImGuiID  SelectedTimeline = 0;

        ImGuiNeoMultiSelector Selector;

        uint32_t CurrentFrame = 0;
        ImVec4   CurrentFrameColor; // Color of current frame, we have to save it because we render on EndNeoSequencer, but process at BeginneoSequencer
        bool     HoldingCurrentFrame = false; // Are we draging current frame?
        bool     KeyFramesEdited = false; // Are key frames dragged?
    };

    IMGUI_API void  RenderNeoSequencerBackground(const ImVec4& color, const ImVec2& cursor, const ImVec2& size, ImDrawList* drawList = nullptr, float sequencerRounding = 0.0f);
    IMGUI_API void  RenderNeoSequencerTopBarBackground(const ImVec4& color, const ImVec2& cursor, const ImVec2& size, ImDrawList* drawList = nullptr, float sequencerRounding = 0.0f);
    IMGUI_API void  RenderNeoSequencerTopBarOverlay(float zoom, float valuesWidth, uint32_t startFrame, uint32_t endFrame, uint32_t offsetFrame, const ImVec2& cursor, const ImVec2& size, ImDrawList* drawList = nullptr, bool drawFrameLines = true, bool drawFrameText = true);
    IMGUI_API void  RenderNeoTimelineLabel(const char* label, const ImVec2& cursor, const ImVec2& size, const ImVec4& color, bool isGroup = false, bool isOpen = false, ImDrawList* drawList = nullptr);
    IMGUI_API void  RenderNeoTimelane(bool selected, const ImVec2& cursor, const ImVec2& size, const ImVec4& highlightColor, ImDrawList* drawList = nullptr);
    IMGUI_API void  RenderNeoTimelinesBorder(const ImVec4& color, const ImVec2& cursor, const ImVec2& size, ImDrawList* drawList = nullptr, float rounding = 0.0f, float borderSize = 1.0f);
    IMGUI_API void  RenderNeoSequencerCurrentFrame(const ImVec4& color, const ImVec4& topColor, const ImRect& pointerBB, float timelineHeight, float lineWidth = 1.0f, ImDrawList* drawList = nullptr);
    IMGUI_API void  RenderSelection(const ImVec4& color, const ImRect& rect, ImDrawList* drawList = nullptr);

    IMGUI_API float GetPerFrameWidth(float totalSizeX, float valuesWidth, uint32_t endFrame, uint32_t startFrame, float zoom);

    IMGUI_API bool  IsItemDoubleClicked(ImGuiMouseButton button);

    float           GetPerFrameWidth(const ImGuiNeoSequencerInternalData& context);
    float           GetKeyframePositionX(uint32_t frame, const ImGuiNeoSequencerInternalData& context);
    ImVec2          GetKeyFramePosition(uint32_t frame, const ImGuiNeoSequencerInternalData& context);
    float           GetWorkTimelineWidth(const ImGuiNeoSequencerInternalData& context);
    // Dont pull frame from context, its used for dragging
    ImRect          GetCurrentFrameBB(uint32_t frame, const ImGuiNeoSequencerInternalData& context, const ImGuiNeoSequencerStyle& style);
    uint32_t        GetCurrentMouseFrame(const ImGuiNeoSequencerInternalData& context);

    // TODO: use non recursive algorithm
    template <typename T>
    bool IsSorted(T arr[], uint32_t n)
    {
        if (n == 1 || n == 0)
            return true;

        if (arr[n - 1] < arr[n - 2])
            return false;

        return IsSorted(arr, n - 1);
    }
}

#endif //IMGUI_NEO_INTERNAL_H

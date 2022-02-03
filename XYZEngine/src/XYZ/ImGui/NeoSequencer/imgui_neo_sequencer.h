//
// Created by Matty on 2022-01-28.
//

#ifndef IMGUI_NEO_SEQUENCER_H
#define IMGUI_NEO_SEQUENCER_H

#include "imgui.h"

typedef int ImGuiNeoSequencerFlags;
typedef int ImGuiNeoSequencerCol;
typedef int ImGuiNeoTimelineFlags;

// Flags for ImGui::BeginNeoSequencer()
enum ImGuiNeoSequencerFlags_
{
    ImGuiNeoSequencerFlags_None                 = 0     ,
    ImGuiNeoSequencerFlags_AllowLengthChanging  = 1 << 0,
    ImGuiNeoSequencerFlags_AllowSelection       = 1 << 1,
    ImGuiNeoSequencerFlags_MouseScroll          = 1 << 2

};

// Flags for ImGui::BeginNeoTimeline()
enum ImGuiNeoTimelineFlags_
{
    ImGuiNeoTimelineFlags_None                 = 0     ,
    ImGuiNeoTimelineFlags_AllowFrameChanging   = 1 << 0,
    ImGuiNeoTimelineFlags_Group                = 1 << 1,
};

enum ImGuiNeoSequencerCol_
{
    ImGuiNeoSequencerCol_Bg,
    ImGuiNeoSequencerCol_TopBarBg,
    ImGuiNeoSequencerCol_SelectedTimeline,
    ImGuiNeoSequencerCol_TimelineBorder,
    ImGuiNeoSequencerCol_TimelinesBg,
    ImGuiNeoSequencerCol_FramePointer,
    ImGuiNeoSequencerCol_FramePointerHovered,
    ImGuiNeoSequencerCol_FramePointerPressed,
    ImGuiNeoSequencerCol_Keyframe,
    ImGuiNeoSequencerCol_KeyframeHovered,
    ImGuiNeoSequencerCol_KeyframeSelected,
    ImGuiNeoSequencerCol_MultiSelectColor,
    ImGuiNeoSequencerCol_COUNT
};

struct ImGuiNeoSequencerStyle 
{
    float       SequencerRounding = 2.5f;       // Corner rounding around whole sequencer
    float       TopBarHeight = 0.0f;            // Value <= 0.0f = Height is calculated by FontSize + FramePadding.y * 2.0f
    bool        TopBarShowFrameLines = true;   // Show line for every frame in top bar
    bool        TopBarShowFrameTexts = true;    // Show frame number every 10th frame
    ImVec2      ItemSpacing = {4.0f,0.5f};
    float       DepthItemSpacing = 10.0f;       // Amount of text offset per depth level in timeline values
    float       TopBarSpacing = 3.0f;           // Space between top bar and timeline
    float       TimelineBorderSize = 1.0f;
    float       CurrentFramePointerSize = 7.0f; // Size of pointing arrow above current frame line
    float       CurrentFrameLineWidth = 1.0f; // Width of line showing current frame over timeline
    float       MouseScrollSpeed = 0.01f; // Speed of zooming with mouse scroll

    ImVec4      Colors[ImGuiNeoSequencerCol_COUNT];

    ImGuiNeoSequencerStyle();
};

struct ImNeoKeyFrame
{
    uint32_t Frame;
    void* UserData;

    bool operator<(const ImNeoKeyFrame& other) const
    {
        return Frame < other.Frame;
    }
    bool operator >(const ImNeoKeyFrame& other) const
    {
        return Frame > other.Frame;
    }
};

/* 
    Key is nullptr if resort happend
    OrigKey and SwapKey are nullptr if only key frame change happend
*/
using ImNeoKeyChangeFn = void(*)(const ImNeoKeyFrame* key);


namespace ImGui {
    IMGUI_API const ImVec4&             GetStyleNeoSequencerColorVec4(ImGuiNeoSequencerCol idx);
    IMGUI_API ImGuiNeoSequencerStyle&   GetNeoSequencerStyle();
    IMGUI_API const ImVector<uint32_t>& GetCurrentTimelineSelection();
   
    IMGUI_API bool                    IsMultiselecting();
    IMGUI_API bool                    IsEditingSelection();
    IMGUI_API bool                    IsCurrentTimelineSelected();
    IMGUI_API bool                    KeyFramesEdited();
    IMGUI_API void                    ClearSelection();

    IMGUI_API ImGuiID                 GetCurrentTimelineID();
    IMGUI_API ImGuiID                 GetSelectedTimelineID();

    IMGUI_API void PushNeoSequencerStyleColor(ImGuiNeoSequencerCol idx, ImU32 col);
    IMGUI_API void PushNeoSequencerStyleColor(ImGuiNeoSequencerCol idx, const ImVec4& col);
    IMGUI_API void PopNeoSequencerStyleColor(int count = 1);

    IMGUI_API bool IsSequencerHovered();
    IMGUI_API bool IsSequencerClicked(ImGuiMouseButton button);



    IMGUI_API bool BeginNeoSequencer(const char* id, uint32_t * frame, uint32_t * startFrame, uint32_t * endFrame, uint32_t* offsetFrame, float *zoom, const ImVec2& size = ImVec2(0, 0), ImGuiNeoSequencerFlags flags = ImGuiNeoSequencerFlags_None);
    IMGUI_API void EndNeoSequencer(); //Call only when BeginNeoSequencer() returns true!!

    IMGUI_API bool BeginNeoGroup(const char* label, bool* open = nullptr);
    IMGUI_API void EndNeoGroup();

    IMGUI_API bool BeginNeoTimeline(const char* label, ImNeoKeyFrame *keyframes, uint32_t keyframeCount, ImNeoKeyChangeFn func, bool * open = nullptr, ImGuiNeoTimelineFlags flags = ImGuiNeoTimelineFlags_None);
    IMGUI_API void EndNeoTimeLine(); //Call only when BeginNeoTimeline() returns true!!

    IMGUI_API bool NeoBeginCreateKeyframe(uint32_t * frame);

    IMGUI_API bool NeoZoomControl(const char* label, float* zoom, uint32_t* offsetFrame, float speed = 0.0001f, bool overlay = false);

}


#endif //IMGUI_NEO_SEQUENCER_H

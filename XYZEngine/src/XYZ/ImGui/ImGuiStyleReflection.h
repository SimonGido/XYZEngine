#pragma once
#include <imgui/imgui.h>

#include <tuple>

namespace XYZ {
	class XYZ_API ImGuiStyleReflection
	{
	public:
		static auto StyleToTuple()
		{
			ImGuiStyle& style = ImGui::GetStyle();
			auto& [e0, e1, e2, e3, e4, e5, e6, e7, e8, e9,
				e10, e11, e12, e13, e14, e15, e16, e17, e18, e19,
				e20, e21, e22, e23, e24, e25, e26, e27, e28, e29,
				e30, e31, e32, e33, e34, e35, e36, e37, e38, e39,
				e40] = style;

			return std::forward_as_tuple(
				e0, e1, e2, e3, e4, e5, e6, e7, e8, e9,
				e10, e11, e12, e13, e14, e15, e16, e17, e18, e19,
				e20, e21, e22, e23, e24, e25, e26, e27, e28, e29,
				e30, e31, e32, e33, e34, e35, e36, e37, e38, e39,
				e40
			);
		}
		
		static constexpr const char* ImGuiColorIDText[] = {
				"ImGuiCol_Text",
				"ImGuiCol_TextDisabled",
				"ImGuiCol_WindowBg",
				"ImGuiCol_ChildBg",
				"ImGuiCol_PopupBg",
				"ImGuiCol_Border",
				"ImGuiCol_BorderShadow",
				"ImGuiCol_FrameBg",
				"ImGuiCol_FrameBgHovered",
				"ImGuiCol_FrameBgActive",
				"ImGuiCol_TitleBg",
				"ImGuiCol_TitleBgActive",
				"ImGuiCol_TitleBgCollapsed",
				"ImGuiCol_MenuBarBg",
				"ImGuiCol_ScrollbarBg",
				"ImGuiCol_ScrollbarGrab",
				"ImGuiCol_ScrollbarGrabHovered",
				"ImGuiCol_ScrollbarGrabActive",
				"ImGuiCol_CheckMark",
				"ImGuiCol_SliderGrab",
				"ImGuiCol_SliderGrabActive",
				"ImGuiCol_Button",
				"ImGuiCol_ButtonHovered",
				"ImGuiCol_ButtonActive",
				"ImGuiCol_Header",
				"ImGuiCol_HeaderHovered",
				"ImGuiCol_HeaderActive",
				"ImGuiCol_Separator",
				"ImGuiCol_SeparatorHovered",
				"ImGuiCol_SeparatorActive",
				"ImGuiCol_ResizeGrip",
				"ImGuiCol_ResizeGripHovered",
				"ImGuiCol_ResizeGripActive",
				"ImGuiCol_Tab",
				"ImGuiCol_TabHovered",
				"ImGuiCol_TabActive",
				"ImGuiCol_TabUnfocused",
				"ImGuiCol_TabUnfocusedActive",
				"ImGuiCol_DockingPreview",
				"ImGuiCol_DockingEmptyBg",
				"ImGuiCol_PlotLines",
				"ImGuiCol_PlotLinesHovered",
				"ImGuiCol_PlotHistogram",
				"ImGuiCol_PlotHistogramHovered",
				"ImGuiCol_TableHeaderBg",
				"ImGuiCol_TableBorderStrong",
				"ImGuiCol_TableBorderLight",
				"ImGuiCol_TableRowBg",
				"ImGuiCol_TableRowBgAlt",
				"ImGuiCol_TextSelectedBg",
				"ImGuiCol_DragDropTarget",
				"ImGuiCol_NavHighlight",
				"ImGuiCol_NavWindowingHighlight",
				"ImGuiCol_NavWindowingDimBg",
				"ImGuiCol_ModalWindowDimBg",
		};

		static constexpr size_t ImGuiStyleVariableCount = 41;

		static constexpr const char* ImGuiStyleVariables[ImGuiStyleVariableCount]{
			"Alpha"                     ,
			"DisabledAlpha"             ,
			"WindowPadding"             ,
			"WindowRounding"            ,
			"WindowBorderSize"          ,
			"WindowMinSize"             ,
			"WindowTitleAlign"          ,
			"WindowMenuButtonPosition"  ,
			"ChildRounding"             ,
			"ChildBorderSize"           ,
			"PopupRounding"             ,
			"PopupBorderSize"           ,
			"FramePadding"              ,
			"FrameRounding"             ,
			"FrameBorderSize"           ,
			"ItemSpacing"               ,
			"ItemInnerSpacing"          ,
			"CellPadding"               ,
			"TouchExtraPadding"         ,
			"IndentSpacing"             ,
			"ColumnsMinSpacing"         ,
			"ScrollbarSize"             ,
			"ScrollbarRounding"         ,
			"GrabMinSize"               ,
			"GrabRounding"              ,
			"LogSliderDeadzone"         ,
			"TabRounding"               ,
			"TabBorderSize"             ,
			"TabMinWidthForCloseButton" ,
			"ColorButtonPosition"       ,
			"ButtonTextAlign"           ,
			"SelectableTextAlign"       ,
			"DisplayWindowPadding"      ,
			"DisplaySafeAreaPadding"    ,
			"MouseCursorScale"          ,
			"AntiAliasedLines"          ,
			"AntiAliasedLinesUseTex"    ,
			"AntiAliasedFill"           ,
			"CurveTessellationTol"	    ,
			"CircleTessellationMaxError",
			"Colors"
		};
	};
}
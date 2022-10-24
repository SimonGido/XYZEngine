#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace XYZ {


	struct ImGuiLink
	{
		ed::LinkId ID;
		ed::PinId  InputID;
		ed::PinId  OutputID;
	};

	
	class ImGuiNodeEditor;
	struct ImGuiNodeValue
	{
		enum ImGuiNodeValueType
		{
			None,
			Float,
			Vec2,
			Vec3,
			Vec4,

			Int,
			IVec2,
			IVec3,
			IVec4,

			Bool,
			NumValues
		};


		void OnImGuiRender(ImGuiNodeEditor* editor);

		ed::PinId InputPinID;
		ed::PinId OutputPinID;

		std::string Name;
		ImGuiNodeValueType Type = ImGuiNodeValueType::None;
	};

	class ImGuiNode
	{
	public:
		void OnImGuiRender(ImGuiNodeEditor* editor);

		std::string					Name;
		ed::NodeId					ID;
		std::vector<ImGuiNodeValue> Values;
	};
}
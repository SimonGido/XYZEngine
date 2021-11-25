#pragma once
#include "XYZ/Renderer/Image.h"

#include <imgui.h>

namespace XYZ {
	namespace UI {
		void Image(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	}
}
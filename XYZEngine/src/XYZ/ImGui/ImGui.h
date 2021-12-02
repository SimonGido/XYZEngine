#pragma once
#include "XYZ/Renderer/Image.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace XYZ {
	namespace UI {
		namespace Utils {

			const char* GenerateID();

		}

		void Image(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
		bool ImageButton(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, int framePadding, const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
		bool ImageButtonTransparent(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, const ImVec4& hoverColor, const ImVec4& clickColor, const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	
		bool BeginTreeNode(const char* name, bool defaultOpen = true);		
		void EndTreeNode();	

		template <typename ...Args>
		bool FloatControl(const char* label, const char* dragLabel, float& value, float resetValue, float speed, Args&& ...args)
		{		
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			const ImVec2 buttonSize = { lineHeight , lineHeight };

			if (ImGui::Button(label, buttonSize))
				value = resetValue;
			ImGui::SameLine();
			bool result = ImGui::DragFloat(dragLabel, &value, speed, 0.0f, 0.0f, "%.2f");

			if constexpr (sizeof... (args) != 0)
			{
				ImGui::SameLine();
				result |= FloatControl(std::forward<Args>(args)...);
			}
			return result;
		}

		template <typename ...Args>
		bool FloatControls(Args&& ...args)
		{
			if constexpr (sizeof... (args) != 0)
			{
				FloatControl(std::forward<Args>(args)...);			
			}
			return true;
		}

		template <typename Value, typename ...Args>
		void TextTableColumns(const char* fmt, Value firstVal, Args&& ...args)
		{
			ImGui::TableNextColumn();
			ImGui::Text(fmt, firstVal);
			if constexpr (sizeof... (args) != 0)
				TextTableColumns(std::forward<Args>(args)...);
		}

		template <typename ...Args>
		void TextTableRow(Args&& ...args)
		{
			ImGui::TableNextRow();
			TextTableColumns(std::forward<Args>(args)...);
		}


		template <typename Func, typename ...Args>
		void TableColumns(Func firstVal, Args&& ...args)
		{
			ImGui::TableNextColumn();
			firstVal();
			if constexpr (sizeof... (args) != 0)
				TableColumns(std::forward<Args>(args)...);
		}

		template <typename ...Args>
		void TableRow(Args&& ...args)
		{
			ImGui::TableNextRow();
			TableColumns(std::forward<Args>(args)...);
		}

		

		class ScopedItemFlags
		{
		public:
			ScopedItemFlags(const ScopedItemFlags&) = delete;
			ScopedItemFlags operator=(const ScopedItemFlags&) = delete;
			ScopedItemFlags(const ImGuiItemFlags flags, const bool enable = true) { ImGui::PushItemFlag(flags, enable); }
			~ScopedItemFlags() { ImGui::PopItemFlag(); }
		};

		class ScopedStyleStack
		{
		public:
			ScopedStyleStack(const ScopedStyleStack&) = delete;
			ScopedStyleStack operator=(const ScopedStyleStack&) = delete;

			template <typename ValueType, typename... OtherStylePairs>
			ScopedStyleStack(ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs&& ... otherStylePairs);
				
			~ScopedStyleStack() { ImGui::PopStyleVar(m_Count); }

		private:

			template <typename ValueType, typename... OtherStylePairs>
			void pushStyle(ImGuiStyleVar styleVar, ValueType value, OtherStylePairs&& ... otherStylePairs);
			
		private:
			int m_Count;
		};

		template<typename ValueType, typename ...OtherStylePairs>
		inline ScopedStyleStack::ScopedStyleStack(ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs && ...otherStylePairs)
			: 
			m_Count((sizeof... (otherStylePairs) / 2) + 1)
		{
			static_assert ((sizeof... (otherStylePairs) & 1u) == 0, "ScopedStyleStack constructor expects a list of pairs of colour IDs and colours as its arguments");
			pushStyle(firstStyleVar, firstValue, std::forward<OtherStylePairs>(otherStylePairs)...);
		}

		template<typename ValueType, typename ...OtherStylePairs>
		inline void ScopedStyleStack::pushStyle(ImGuiStyleVar styleVar, ValueType value, OtherStylePairs && ...otherStylePairs)
		{
			ImGui::PushStyleVar(styleVar, value);
			if constexpr (sizeof... (otherStylePairs) != 0)
				pushStyle(std::forward<OtherStylePairs>(otherStylePairs)...);
		}


		class ScopedColorStack
		{
		public:
			ScopedColorStack(const ScopedColorStack&) = delete;
			ScopedColorStack operator=(const ScopedColorStack&) = delete;

			template <typename ColorType, typename... OtherColors>
			ScopedColorStack(ImGuiCol firstColorID, ColorType firstColor, OtherColors&& ... otherColorPairs);
				
			~ScopedColorStack() { ImGui::PopStyleColor(m_Count); }

		private:
			template <typename ColorType, typename... OtherColors>
			void pushColor(ImGuiCol colorID, ColorType color, OtherColors&& ... otherColorPairs);
		
		private:
			int m_Count;
		};
		template<typename ColorType, typename ...OtherColors>
		inline ScopedColorStack::ScopedColorStack(ImGuiCol firstColorID, ColorType firstColor, OtherColors && ...otherColorPairs)
			:
			m_Count((sizeof... (otherColorPairs) / 2) + 1)
		{
			static_assert ((sizeof... (otherColorPairs) & 1u) == 0,"ScopedColorStack constructor expects a list of pairs of color IDs and colors as its arguments");
			pushColor(firstColorID, firstColor, std::forward<OtherColors>(otherColorPairs)...);
		}
		template<typename ColorType, typename ...OtherColors>
		inline void ScopedColorStack::pushColor(ImGuiCol colorID, ColorType color, OtherColors && ...otherColorPairs)
		{
			ImGui::PushStyleColor(colorID, color);
			if constexpr (sizeof... (otherColorPairs) != 0)
				pushColor(std::forward<OtherColors>(otherColorPairs)...);
		}
	}
}
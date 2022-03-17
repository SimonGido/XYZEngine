#pragma once
#include "XYZ/Renderer/Image.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace XYZ {
	namespace UI {
		namespace Utils {
			const char* GenerateID();
			void        SetPathBuffer(const char* value, size_t size);
			void	    SetPathBuffer(const std::string& value);
			char*	    GetPathBuffer();
			void		PushID();
			void		PopID();

			float Convert_Linear_ToSRGB(float theLinearValue);
			
			float Convert_SRGB_ToLinear(float thesRGBValue);
			
			ImVec4 ConvertToSRGB(const ImVec4& colour);
			
			ImVec4 ConvertToLinear(const ImVec4& colour);	

			bool IsItemDoubleClicked(ImGuiMouseButton button);
		}

		void HelpMarker(const char* desc);
		
		void Image(const Ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
		bool ImageButton(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, int framePadding, const ImVec4& bgColor = ImVec4(1, 1, 1, 1), const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
		bool ImageButtonTransparent(const char* stringID, const Ref<Image2D>& image, const ImVec2& size, const ImVec4& hoverColor, const ImVec4& clickColor, const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	
		bool BeginTreeNode(const char* name, bool defaultOpen = true);		
		void EndTreeNode();	

		

		bool Vec2Control(const std::array<const char*, 2>& names, glm::vec2& values, float resetValue = 0.0f, float speed = 0.05f);
		bool Vec3Control(const std::array<const char*, 3>& names, glm::vec3& values, float resetValue = 0.0f, float speed = 0.05f);
		bool Vec4Control(const std::array<const char*, 4>& names, glm::vec4& values, float resetValue = 0.0f, float speed = 0.05f);

		bool IVec2Control(const std::array<const char*, 2>& names, glm::ivec2& values, float resetValue = 0.0f, float speed = 0.05f);
		bool IVec3Control(const std::array<const char*, 3>& names, glm::ivec3& values, float resetValue = 0.0f, float speed = 0.05f);
		bool IVec4Control(const std::array<const char*, 4>& names, glm::ivec4& values, float resetValue = 0.0f, float speed = 0.05f);
		


		template <size_t numHeaders, typename T, typename Func>
		void ContainerControl(const char* name, T& container, const std::array<const char*, numHeaders>& headerNames, size_t& selectedIndex, Func&& elementControl)
		{
			const size_t oldSelectedIndex = selectedIndex;
			if (ImGui::TreeNode(name))
			{
				//ImGui::Text(name);
				if (ImGui::BeginTable(name, numHeaders, ImGuiTableFlags_SizingStretchSame))
				{
					for (auto& header : headerNames)
						ImGui::TableSetupColumn(header);
					ImGui::TableHeadersRow();

					size_t index = 0;
					for (auto& el : container)
					{
						elementControl(el, selectedIndex, index);
						index++;
					}
					ImGui::EndTable();
				}

				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, glm::vec2(0.0f));
				if (ImGui::Button("+"))
					container.push_back({});
				ImGui::SameLine();
				if (ImGui::Button("-"))
				{
					if (selectedIndex < container.size())
					{
						container.erase(container.begin() + selectedIndex);
						selectedIndex = SIZE_MAX;
					}
					else if (!container.empty())
					{
						container.pop_back();
					}
				}
				// Mouse released and no new selection happend
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && oldSelectedIndex == selectedIndex)
				{
					selectedIndex = SIZE_MAX;
				}
				ImGui::TreePop();
			}
		}


		template <typename Func0, typename Func1>
		void SplitterV(float* size, const char* stringID0, const char* stringID1, const Func0& func0, const Func1& func1)
		{
			ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
			ScopedStyleStack styleSpacing(true, ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::BeginChild(stringID0, ImVec2(*size, 0), true);
			
			{
				ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, spacing);
				func0();
			}
			
			float h = ImGui::GetWindowHeight();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::InvisibleButton("splitterButton", ImVec2(8.0f, h));
			if (ImGui::IsItemActive())
				*size += ImGui::GetIO().MouseDelta.x;

			ImGui::SameLine();
		
			ImGui::BeginChild(stringID1, ImVec2(0, 0), true);			
			{
				ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, spacing);
				func1();
			}
			ImGui::EndChild();	
		}

		template <typename ...Args>
		void Toolbar(glm::vec2 offset, glm::vec2 spacing, bool vertical, Args&& ...args)
		{
			const ImVec2 oldCursorPos = ImGui::GetCursorPos();
			const ImVec2 newCursorPos = {
				ImGui::GetWindowContentRegionMin().x + offset.x,
				ImGui::GetWindowContentRegionMin().y + offset.y
			};
			ImGui::SetCursorPos(newCursorPos);
			auto sameLineFunc = [](auto callable, size_t& counter) {
				callable();
				if (counter > 1)
					ImGui::SameLine();
				counter--;
			};

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
			if (vertical)
			{
				(args(), ...);
			}
			else
			{
				size_t counter = sizeof... (args);
				(sameLineFunc(std::forward<Args>(args), counter), ...);
			}
			ImGui::PopStyleVar();
			ImGui::SetCursorPos(oldCursorPos);
		}

		template <typename T>
		bool DragDropSource(const char* type, const T* data, size_t size, ImGuiDragDropFlags dragFlags = ImGuiDragDropFlags_SourceAllowNullID, ImGuiCond flags = ImGuiCond_Once)
		{
			if (ImGui::BeginDragDropSource(dragFlags))
			{
				ImGui::SetDragDropPayload(type,(const void*)data, size, flags);
				ImGui::EndDragDropSource();
				return true;
			}
			return false;
		}

		template <typename T>
		bool DragDropTarget(const char* type, T* out, ImGuiDragDropFlags flags = 0)
		{
			bool result = false;
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type, flags))
				{
					*out = (T)payload->Data;
					result = true;
				}
				ImGui::EndDragDropTarget();
			}
			return result;
		}

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
			bool result = false;
			if constexpr (sizeof... (args) != 0)
			{
				result |= FloatControl(std::forward<Args>(args)...);			
			}
			return result;
		}

		template <typename ...Args>
		bool IntControl(const char* label, const char* dragLabel, int& value, int resetValue, float speed, Args&& ...args)
		{
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			const ImVec2 buttonSize = { lineHeight , lineHeight };

			if (ImGui::Button(label, buttonSize))
				value = resetValue;
			ImGui::SameLine();
			bool result = ImGui::DragInt(dragLabel, &value, speed, 0, 0, "%d");
			if constexpr (sizeof... (args) != 0)
			{
				ImGui::SameLine();
				result |= IntControl(std::forward<Args>(args)...);
			}
			return result;
		}

		template <typename ...Args>
		bool IntControls(Args&& ...args)
		{
			bool result = false;
			if constexpr (sizeof... (args) != 0)
			{
				result |= IntControl(std::forward<Args>(args)...);
			}
			return result;
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
		void TableRow(const void* stringID, Args&& ...args)
		{
			ImGui::PushID(stringID);
			ImGui::TableNextRow();
			TableColumns(std::forward<Args>(args)...);
			ImGui::PopID();
		}

		class ScopedTableColumnAutoWidth
		{
		public:
			ScopedTableColumnAutoWidth(uint32_t itemCount, float offset = 0.0f)
			{
				auto table = ImGui::GetCurrentTable();
				auto& column = table->Columns[table->CurrentColumn];
				const float width = column.WorkMaxX - column.WorkMinX;
				ImGui::PushItemWidth((width / static_cast<float>(itemCount)) - offset);
			}
			~ScopedTableColumnAutoWidth()
			{
				ImGui::PopItemWidth();
			}
		};

		class ScopedID
		{
		public:
			template <typename T>
			ScopedID(T id) { ImGui::PushID(id); }
			~ScopedID() { ImGui::PopID(); }
		};

		class ScopedWidth
		{
		public:
			ScopedWidth(float width){ ImGui::PushItemWidth(width);}
			~ScopedWidth() { ImGui::PopItemWidth(); }
		};

		class ScopedItemFlags
		{
		public:
			ScopedItemFlags(const ScopedItemFlags&) = delete;
			ScopedItemFlags operator=(const ScopedItemFlags&) = delete;
			ScopedItemFlags(const ImGuiItemFlags flags, const bool enable = true) 
				: m_Enable(enable) { if (m_Enable) ImGui::PushItemFlag(flags, true); }
			~ScopedItemFlags() { if (m_Enable) ImGui::PopItemFlag(); }

		private:
			const bool m_Enable;
		};


		class ScopedStyleStack
		{
		public:
			ScopedStyleStack(const ScopedStyleStack&) = delete;
			ScopedStyleStack operator=(const ScopedStyleStack&) = delete;

			template <typename ValueType, typename... OtherStylePairs>
			ScopedStyleStack(bool push, ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs&& ... otherStylePairs);
				
			~ScopedStyleStack() { if (m_Push) ImGui::PopStyleVar(m_Count); }

		private:

			template <typename ValueType, typename... OtherStylePairs>
			void pushStyle(ImGuiStyleVar styleVar, ValueType value, OtherStylePairs&& ... otherStylePairs);
			
		private:
			int m_Count;
			bool m_Push;
		};

		template<typename ValueType, typename ...OtherStylePairs>
		inline ScopedStyleStack::ScopedStyleStack(bool push, ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs && ...otherStylePairs)
			: 
			m_Count((sizeof... (otherStylePairs) / 2) + 1),
			m_Push(push)
		{
			static_assert ((sizeof... (otherStylePairs) & 1u) == 0, "ScopedStyleStack constructor expects a list of pairs of colour IDs and colours as its arguments");
			if (m_Push)
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
			ScopedColorStack(bool push, ImGuiCol firstColorID, ColorType firstColor, OtherColors&& ... otherColorPairs);
				
			~ScopedColorStack() { if (m_Push) ImGui::PopStyleColor(m_Count); }

		private:
			template <typename ColorType, typename... OtherColors>
			void pushColor(ImGuiCol colorID, ColorType color, OtherColors&& ... otherColorPairs);
		
		private:
			int m_Count;
			bool m_Push;
		};
		template<typename ColorType, typename ...OtherColors>
		inline ScopedColorStack::ScopedColorStack(bool push, ImGuiCol firstColorID, ColorType firstColor, OtherColors && ...otherColorPairs)
			:
			m_Count((sizeof... (otherColorPairs) / 2) + 1),
			m_Push(push)
		{
			static_assert ((sizeof... (otherColorPairs) & 1u) == 0,"ScopedColorStack constructor expects a list of pairs of color IDs and colors as its arguments");
			if (m_Push)
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
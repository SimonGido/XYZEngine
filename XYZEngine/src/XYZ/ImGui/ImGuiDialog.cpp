#include "stdafx.h"
#include "ImGuiDialog.h"

#include <imgui/imgui.h>

namespace XYZ {
	void ImGuiDialogQueue::Push(std::string name, DialogFunc&& func)
	{
		m_Dialogs.push(DialogContext{ std::move(name), std::move(func), true });
	}
	void ImGuiDialogQueue::Pop()
	{
		m_Dialogs.pop();
	}
	void ImGuiDialogQueue::Update()
	{
		if (!m_Dialogs.empty())
		{
			bool finished = false;
			auto& last = m_Dialogs.back();

			if (ImGui::Begin(last.Name.c_str(), &last.Open, ImGuiWindowFlags_NoDocking))
			{				
				finished = last.Func();
			}
			ImGui::End();

			if (finished || !last.Open)
				m_Dialogs.pop();
		}
	}
}
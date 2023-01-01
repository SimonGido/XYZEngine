#pragma once
#include <queue>
#include <deque>

namespace XYZ {
	class XYZ_API ImGuiDialogQueue
	{
	public:
		using DialogFunc = std::function<bool()>;

		void Push(std::string name, DialogFunc&& func);

		void Pop();

		void Update();

	private:

		struct DialogContext
		{
			std::string Name;
			DialogFunc  Func;
			bool		Open;
		};

		std::queue<DialogContext> m_Dialogs;
	};
}
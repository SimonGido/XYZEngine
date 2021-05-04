#pragma once
#include "XYZ/BasicUI/BasicUI.h"
#include "XYZ/Core/Timestep.h"


namespace XYZ {
	namespace Editor {
		class EditorUI
		{
		public:
			EditorUI(const std::string& filepath);
			virtual ~EditorUI();

			virtual void OnUpdate(Timestep ts) = 0;
			virtual void OnReload() = 0;
			virtual void SetupUI() = 0;

			const std::string& GetFilepath() const { return m_Filepath; }
			const std::string& GetName() const { return m_Name; }
		protected:
			const std::string m_Filepath;
			const std::string m_Name;
		};
	}
}
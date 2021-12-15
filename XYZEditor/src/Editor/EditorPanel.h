#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Scene/Scene.h"

namespace XYZ {
	namespace Editor {
		class EditorPanel : public RefCount
		{
		public:
			EditorPanel(std::string name);

			virtual void OnImGuiRender(bool& open) = 0;
			virtual void OnUpdate(Timestep ts) {};
			virtual bool OnEvent(Event& e) { return false; };

			virtual void SetSceneContext(const Ref<Scene>& scene) {};
			const std::string& GetName() const { return m_Name; }

		protected:
			const std::string m_Name;
		};
	}
}
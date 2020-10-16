#pragma once
#include <XYZ.h>

#include "../Tools/EditorCamera.h"

namespace XYZ {

	class GraphLayout
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};
	};

	class GraphPanel
	{
	public:
		GraphPanel();

		bool OnInGuiRender(Timestep ts);
		void SetGraphLayout(GraphLayout* layout) { m_Layout = layout; }
		void OnEvent(Event& event);
	private:
		GraphLayout* m_Layout = nullptr;
		InGuiWindow* m_GraphWindow = nullptr;

		glm::vec2 m_TestPosition = { 0,0 };

		bool m_ActiveWindow = false;
		const uint32_t m_GraphID = 3;
	};
}
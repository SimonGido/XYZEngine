#pragma once

#include "XYZ/Core/Layer.h"
#include "XYZ/Core/Event/Event.h"

namespace XYZ
{
	/*! @class ImGuiLayer
	*	@brief Layer for handling ImGUI events
	*/
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin(const float ts);
		void End();

	private:
		bool OnMouseButtonPressedEvent(event_ptr e);
		bool OnMouseButtonReleasedEvent(event_ptr e);
		bool OnMouseMovedEvent(event_ptr e);
		bool OnMouseScrolledEvent(event_ptr e);
		bool OnWindowResizedEvent(event_ptr e);
		bool OnKeyPressedEvent(event_ptr e);
		bool OnKeyReleasedEvent(event_ptr e);
		bool OnKeyTypedEvent(event_ptr e);
	};
};
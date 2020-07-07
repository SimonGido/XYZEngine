#pragma once
#include "XYZ/Event/Event.h"



namespace XYZ {


	class LayerStack;

	/*! @class Layer
	*	@brief Layers are used to handle events
	*/


	class Layer
	{
		friend class LayerStack;
	public:
		Layer();
		virtual ~Layer();

		virtual void OnAttach() {} //Init
		virtual void OnDetach() {} //Destroy
		virtual void OnUpdate(float ts) {} //Update
		virtual void OnEvent(Event& event) {};
		


	protected:

		int32_t m_Key;
	};

}

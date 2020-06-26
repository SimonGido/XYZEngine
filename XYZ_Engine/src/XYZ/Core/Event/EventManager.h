#pragma once
#include "Event.h"
#include "XYZ/Core/Singleton.h"

#include <map>
#include <functional>

namespace XYZ {
	using HandlerID = unsigned int;
	typedef std::function<void(event_ptr)> handlerPtr;
	typedef std::map<EventType, std::vector<std::pair<unsigned int, handlerPtr>>> handler_map;

	/*! @class EventManager
	*	@brief Manages adding, removing and firing events
	*/
	class EventManager : public Singleton<EventManager>
	{
	public:
		EventManager(token) : m_Handlers(handler_map()), m_NextId(0)
		{}

		bool FireEvent(event_ptr event);
		HandlerID AddHandler(EventType type, handlerPtr eventHandler);
		bool RemoveHandler(EventType type, HandlerID handlerId);
		void RemoveAllHandlers(EventType type);

	private:

		handler_map m_Handlers;
		unsigned int m_NextId;


	};

}

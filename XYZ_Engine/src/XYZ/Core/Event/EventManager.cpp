#include "stdafx.h"
#include "EventManager.h"

namespace XYZ {
	//Add an event to manager
	bool EventManager::FireEvent(event_ptr event)
	{
		auto type = event->GetEventType();

		if (m_Handlers.find(type) == m_Handlers.end())
			return false;

		auto evHandlers = m_Handlers[type];

		//Invoke handlers starting at the end of the handler vector, stop once the event becomes handled
		for (auto i = evHandlers.rbegin(); !event->IsHandled() && i != evHandlers.rend(); ++i)
			(*i).second(event);

	}

	//Attach handler to an event
	HandlerID EventManager::AddHandler(EventType type, handlerPtr eventHandler)
	{
		//Initialize a new map of handlers for the given event name
		if (m_Handlers.find(type) == m_Handlers.end())
			m_Handlers[type] = std::vector<std::pair<unsigned int, handlerPtr>>();

		//Increment next event ID and push back handler
		auto assignedId = m_NextId++;
		m_Handlers[type].push_back(std::make_pair(assignedId, eventHandler));


		return assignedId;
	}

	//Remove handler from an event
	bool EventManager::RemoveHandler(EventType type, HandlerID handlerId)
	{
		if (m_Handlers.find(type) == m_Handlers.end())
			return false;

		//Find an element that matches the ID
		auto handler = std::find_if(m_Handlers[type].begin(), m_Handlers[type].end(), [&handlerId](const std::pair<unsigned int, handlerPtr>& element) { return element.first == handlerId; });
		if (handler == m_Handlers[type].end())
			return false;

		m_Handlers[type].erase(handler);

		return true;
	}

	//Removes all handlers from an event
	void EventManager::RemoveAllHandlers(EventType type)
	{
		if (m_Handlers.find(type) == m_Handlers.end())
			m_Handlers[type] = std::vector<std::pair<unsigned int, handlerPtr>>();
		else
			m_Handlers[type].clear();
	}

}
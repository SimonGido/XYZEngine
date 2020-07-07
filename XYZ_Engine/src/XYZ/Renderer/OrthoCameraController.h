#pragma once

#include "XYZ/Renderer/OrthoCamera.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/ApplicationEvent.h"


namespace XYZ {

	

	/**
	* @class OrthoCameraController
	*/
	class OrthoCameraController
	{
	public:
		/**
		* Construct an orthographic camera controller ,an orthohraphic camera from the specified aspect ratio.
		* Add handlers for events to the Event Manager
		* @param[in] aspectRatio
		* @param[in] rotation default false
		*/
		OrthoCameraController(float aspectRatio, bool rotation = false);
		~OrthoCameraController();

		/**
		* Updates the camera position and rotation dependent on the input.
		* To control position use WASD keys. To control rotation use QE keys, 
		* rotation argument in the constructor must be set to true.
		* Movement and rotation speed is multiplied by dt 
		* @param[in] dt		Duration of the last frame
		*/
		void OnUpdate(float dt);

		void OnEvent(Event& event);
		/**
		* @return reference to the orthocamera
		*/
		OrthoCamera& GetCamera() { return m_Camera; }

		/**
		* @return const reference to the orthocamera
		*/
		const OrthoCamera& GetCamera() const { return m_Camera; }

		/**
		* @return zoom level of the camera
		*/
		float GetZoomLevel() const { return m_ZoomLevel; }
		/**
		* Set the zoom level of the camera
		* @param[in] level
		*/
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

	private:
		/**
		* Handler for the mouse scrolled event
		* @param[in] event shared_ptr to the Event
		*/
		void OnMouseScrolled(MouseScrollEvent& event);

		/**
		* Handler for the window resized event
		* @param[in] event shared_ptr to the Event
		*/
		void OnWindowResized(WindowResizeEvent& event);
	private:
		
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthoCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};

}
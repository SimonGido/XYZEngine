#pragma once
#include "Framebuffer.h"

#include <glm/glm.hpp>


namespace XYZ {

	/**
	* @class OrthoCameraBound
	* @brief represents boundaries of view frustum of the camera
	*/
	struct OrthoCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		/**
		* @return width
		*/
		float GetWidth() const { return Right - Left; }
		/**
		* @return height
		*/
		float GetHeight() const { return Top - Bottom; }
	};

	/**
	* @class OrthoCamera
	* @brief represents camera for 2D scene, calculates projection and view matrix 
	*/
	class OrthoCamera
	{
	public:
		/**
		* Constructor, takes bounds of view frustrum of the camera
		* @param[in] left
		* @param[in] right
		* @param[in] bottom
		* @param[in] top
		*/
		OrthoCamera(float left, float right, float bottom, float top);

		/**
		* If render target not null bind it
		*/
		void Bind();

		/**
		* If render target not null unbind it
		*/
		void UnBind();
		/**
		* @return position of the camera
		*/
		const glm::vec3& GetPosition() const { return m_Position; }


		/**
		* @param[in] frameBuffer
		* Set render targer of camera
		*/
		inline void SetRenderTarget(std::shared_ptr<FrameBuffer> frameBuffer) { m_RenderTarget = frameBuffer; };
		/**
		* Set position of the camera and recalculate the view matrix
		* @param[in] pos 
		*/
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; recalculateViewMatrix(); }

		/**
		* @return rotation of the camera
		*/
		inline float GetRotation() const { return m_Rotation; }

		/**
		* Set the rotation of the camera and recalculate the view matrix
		* @param[in] rot
		*/
		inline void SetRotation(float rot) { m_Rotation = rot; recalculateViewMatrix(); }

		/**
		* Set projection matrix dependent on boundaries
		* @param[in] left
		* @param[in] right
		* @param[in] bottom
		* @param[in] top
		*/
		void SetProjection(float left, float right, float bottom, float top);

		/**
		* @return projection matrix of the camera
		*/
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		/**
		* @return view matrix of the camera
		*/
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

		/**
		* @return projectionview matrix of the camera
		*/
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		/**
		* @return bounds of view frustum of the camera
		*/
		const OrthoCameraBounds& GetBounds() const { return m_Bounds; }
	private:
		/**
		* Recalculate view matrix of the camera dependent on it's position and rotation
		*/
		void recalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		float m_Rotation = 0.0f;

		OrthoCameraBounds m_Bounds;
		std::shared_ptr<FrameBuffer> m_RenderTarget;
	};

}
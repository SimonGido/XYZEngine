#pragma once

#include "XYZ/Core/Core.h"


namespace XYZ {
	class XYZ_API Timestep
	{
	public:
		Timestep() {}
		Timestep(float time) : m_Time(time) {}

		inline float GetSeconds() const { return m_Time; }
		inline float GetMilliseconds() const { return m_Time * 1000.0f; }

		operator float() { return m_Time; }

		Timestep operator +=(const Timestep& other)
		{
			m_Time += other.m_Time;
			return *this;
		}
	private:
		float m_Time = 0.0f;
	};


}
#pragma once
#include "XYZ/Utils/DataStructures/DynamicPool.h"

namespace XYZ {

	enum class PropertyType
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
		Sprite
	};

	class IProperty : public DynamicPool::Base
	{
	public:
		IProperty(PropertyType type);

		virtual ~IProperty() = default;
		virtual void Update(float time) = 0;
		virtual float GetLength() const = 0;

		PropertyType GetType() const { return m_Type; }
		

	protected:
		size_t m_CurrentFrame = 0;
		const PropertyType m_Type;
	};

	template <typename T>
	class Property : public IProperty					 				
	{
	public:
		Property(T& value);
		Property(const Property<T>& other);
				
		virtual void Update(float time) override;
		
		virtual float GetLength() const override 
		{
			return m_KeyFrames.back().EndTime;
		}
		void AddKeyFrame(T value, float endTime)
		{
			KeyFrame key{value, endTime};
			m_KeyFrames.insert(std::upper_bound(m_KeyFrames.begin(), m_KeyFrames.end(), key), key);
		}
		void RemoveKeyFrame(size_t index)
		{
			m_KeyFrames.erase(m_KeyFrames.begin() + index);
		}

		virtual void OnCopy(uint8_t* buffer) override
		{
			new(buffer)Property<T>(*this);
		}

	private:
		struct KeyFrame
		{
			bool operator <(const KeyFrame& other) const
			{
				return EndTime < other.EndTime;
			}
			bool operator >(const KeyFrame& other) const
			{
				return EndTime > other.EndTime;
			}
			T Value;
			float EndTime;
		};
		static T interpolate(const KeyFrame& begin, const KeyFrame& end, float time);	

	private:
		T& m_Value;
		std::vector<KeyFrame> m_KeyFrames;

		friend class Animation;
	};


	class Animation
	{
	public:
		void Update(float dt);
		void SetLength(float length);

		template <typename T, typename ...Args>
		Property<T>& EmplaceProperty(Args&& ...args)
		{
			m_Properties.Emplace(std::forward<Args>(args)...);
			updateLength();
			return m_Properties.Back();
		}

		template <typename T>
		Property<T>& PushProperty(const T& other)
		{
			m_Properties.Push(other);
			updateLength();
			return m_Properties.Back();
		}

		void RemoveProperty(size_t index)
		{
			m_Properties.Erase(index);
			updateLength();
		}

		size_t GetNumberOfProperties() const { return m_Properties.Size(); }

	private:
		void updateLength();

	private:
		DynamicPool m_Properties;

		float m_Length;
		float m_CurrentTime = 0.0f;
		bool m_Repeat = true;
	};
	
}
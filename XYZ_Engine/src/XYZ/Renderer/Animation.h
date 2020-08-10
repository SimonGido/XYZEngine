#pragma once


namespace XYZ {

	struct IProperty
	{
		virtual void Transition(float currentTime) = 0;

		size_t CurrentFrame = 0;
	};

	template <typename T>
	struct Property : public IProperty
	{
		Property(T& modified)
			: ModifiedValue(modified)
		{}
			
		virtual void Transition(float currentTime) override;

		struct KeyFrame
		{
			T Value;
			float Time;
		};

		T& ModifiedValue;
		std::vector<KeyFrame> KeyFrames;
	};

	class Animation
	{
	public:
		Animation(float animLength,  bool repeat = true);
		~Animation();

		void Update(float dt);
		
		void AddProperty(IProperty* prop) { m_Properties.push_back(prop); }

		float m_CurrentTime = 0.0f;
		float m_AnimationLength;
		bool m_Repeat;

	private:
		std::vector<IProperty*> m_Properties;
	};
}
#pragma once


namespace XYZ {

	template <typename T>
	struct KeyFrame
	{
		T Value;
		float Time;
	};

	struct IProperty
	{
		virtual void Transition(float time, uint32_t fps) = 0;

		size_t CurrentFrame = 0;
	};

	template <typename T>
	struct Property : public IProperty
	{
		virtual void Transition(float time, uint32_t fps) override;

		T* ModifiedValue = nullptr;;
		std::vector<KeyFrame<T>> KeyFrames;
	};

	class Animation
	{
	public:
		Animation(float animLength, uint32_t fps, bool repeat = true);
		~Animation();

		void Update(float dt);
		
		void AddProperty(IProperty* prop) { m_Properties.push_back(prop); }

		float m_CurrentTime = 0.0f;
		float m_AnimationLength;
		uint32_t m_FPS;
		bool m_Repeat;

	private:
		std::vector<IProperty*> m_Properties;
	};
}
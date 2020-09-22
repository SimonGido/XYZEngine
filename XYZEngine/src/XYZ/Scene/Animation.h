#pragma once


namespace XYZ {

	struct IProperty
	{
		virtual void Update(float currentTime) = 0;

		size_t CurrentFrame = 0;
	};

	template <typename T>
	struct Property : public IProperty
	{
		Property(T& modified)
			: ModifiedValue(modified)
		{}
			
		virtual void Update(float currentTime) override;

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
		
		template <typename T, typename ...Args>
		Property<T>* AddProperty(Args&&... args)
		{
			auto prop = new Property<T>(std::forward<Args>(args)...);
			Properties.emplace_back(prop);
			return prop;
		}

		float CurrentTime = 0.0f;
		float AnimationLength;
		bool Repeat;

		std::vector<IProperty*> Properties;
	};
}
#include "Track.h"


namespace XYZ {
	class TransformTrack : public Track
	{
	public:
		enum class PropertyType { Translation, Rotation, Scale };

		TransformTrack(SceneEntity entity);

		virtual bool  Update(float time) override;
		virtual void  Reset() override;
		virtual float Length() override;


		template <typename T>
		void AddKeyFrame(const KeyFrame<T>& key, PropertyType type);

	private:
		Property<glm::vec3> m_TranslationProperty;
		Property<glm::vec3> m_RotationProperty;
		Property<glm::vec3> m_ScaleProperty;
	};
}
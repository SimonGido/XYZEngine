#include "Track.h"


namespace XYZ {
	class TransformTrack : public Track
	{
	public:
		enum class PropertyType { Translation, Rotation, Scale };

		TransformTrack(SceneEntity entity);

		virtual bool  Update(uint32_t frame) override;
		virtual void  Reset() override;
		virtual uint32_t Length() const override;

		void AddKeyFrame(const KeyFrame<glm::vec3>& key, PropertyType type);
		void RemoveKeyFrame(uint32_t frame, PropertyType type);
	private:
		virtual void updatePropertyCurrentKey(uint32_t frame) override;

	private:
		Property<glm::vec3> m_TranslationProperty;
		Property<glm::vec3> m_RotationProperty;
		Property<glm::vec3> m_ScaleProperty;
	};
}
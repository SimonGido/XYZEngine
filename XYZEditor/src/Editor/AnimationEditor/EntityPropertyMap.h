#pragma once
#include "XYZ/Animation/Animation.h"
#include "XYZ/ImGui/NeoSequencer/imgui_neo_sequencer.h"

namespace XYZ {
	namespace Editor {

		class EntityPropertyMap
		{
		public:
			struct PropertyData
			{
				IProperty*				   Property;
				std::vector<ImNeoKeyFrame> KeyFrames;
				ImNeoKeyChangeFn		   KeyChangeFunc;
			};
			using DataMap = std::unordered_map<std::string, 
							std::unordered_map<std::string, std::vector<PropertyData>>>;
			
			using iterator = DataMap::iterator;
			using const_iterator = DataMap::const_iterator;


			void BuildMap(Ref<Animation>& anim);

			iterator	    begin()		   { return m_PropertyData.begin(); }
			iterator	    end()		   { return m_PropertyData.end(); }
			const_iterator  cbegin() const { return m_PropertyData.cbegin(); }
			const_iterator  cend()	 const { return m_PropertyData.cend(); }

		private:
			template <typename T>
			void processPropertyType(std::vector<Property<T>>& props);


			template <typename T>
			static void keyChangeFunc(const ImNeoKeyFrame* key, const ImNeoKeyFrame* origKey, const ImNeoKeyFrame* swapKey);

		private:
			// Entity Name -> Component Name -> Properties
			DataMap m_PropertyData;

		};
		template<typename T>
		inline void EntityPropertyMap::processPropertyType(std::vector<Property<T>>& props)
		{
			for (auto& prop : props)
			{
				auto& entityData = m_PropertyData[prop.GetPath()];
				auto& componentData = entityData[prop.GetComponentName()];

				componentData.push_back({ &prop, {}, &keyChangeFunc<T> });
				auto& keyFrames = componentData.back().KeyFrames;

				for (auto& keyFrame : prop.Keys)
				{
					keyFrames.push_back({ keyFrame.Frame, &keyFrame });
				}
			}
		}
		template<typename T>
		inline void EntityPropertyMap::keyChangeFunc(const ImNeoKeyFrame* key, const ImNeoKeyFrame* origKey, const ImNeoKeyFrame* swapKey)
		{
			// TODO: This is not the most elegant way to do this;
			const bool resortHappend = origKey && swapKey;
			if (resortHappend) // Resort happend, so it is necesary to swap two keys
			{
				KeyFrame<T>* userOrigKey = static_cast<KeyFrame<T>*>(origKey->UserData);
				KeyFrame<T>* userSwapKey = static_cast<KeyFrame<T>*>(swapKey->UserData);
				userOrigKey->Frame = swapKey->Frame;
				userSwapKey->Frame = origKey->Frame;
				std::swap(*userOrigKey, *userSwapKey);
			}
			else // The key was modified
			{
				KeyFrame<T>* userKey = static_cast<KeyFrame<T>*>(key->UserData);
				userKey->Frame = key->Frame;
			}
		}
	}
}
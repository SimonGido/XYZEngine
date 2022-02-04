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
				bool					   Open = true;
			};

			struct ComponentData
			{
				std::vector<PropertyData> Properties;
				bool Open = true;
			};

			struct EntityData
			{
				std::unordered_map<std::string, ComponentData> Data;
				bool Open = true;
			};

			using DataMap = std::unordered_map<std::string, EntityData>;
			
			using iterator = DataMap::iterator;
			using const_iterator = DataMap::const_iterator;
		public:

			void BuildMap(Ref<Animation>& anim);
			void Clear()	   { m_PropertyData.clear(); }
			bool Empty() const { return m_PropertyData.empty(); }

			iterator	    begin()		   { return m_PropertyData.begin(); }
			iterator	    end()		   { return m_PropertyData.end(); }
			const_iterator  cbegin() const { return m_PropertyData.cbegin(); }
			const_iterator  cend()	 const { return m_PropertyData.cend(); }

		private:
			template <typename T>
			void processPropertyType(std::vector<Property<T>>& props);


			template <typename T>
			static void keyChangeFunc(const ImNeoKeyFrame* key);

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
				auto& componentData = entityData.Data[prop.GetComponentName()];

				componentData.Properties.push_back({ &prop, {}, &keyChangeFunc<T>, true });
				auto& keyFrames = componentData.Properties.back().KeyFrames;

				for (auto& keyFrame : prop.Keys)
				{
					keyFrames.push_back({ keyFrame.Frame, &keyFrame });
				}
			}
		}
		template<typename T>
		inline void EntityPropertyMap::keyChangeFunc(const ImNeoKeyFrame* key)
		{
			KeyFrame<T>* userKey = static_cast<KeyFrame<T>*>(key->UserData);
			userKey->Frame = key->Frame;
		}
	}
}
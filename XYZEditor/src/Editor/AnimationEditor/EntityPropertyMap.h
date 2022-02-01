#pragma once
#include "XYZ/Animation/Animation.h"

namespace XYZ {
	namespace Editor {

		class EntityPropertyMap
		{
		public:
			struct PropertyData
			{
				IProperty* Property;
				std::vector<uint32_t> KeyFrames;
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

				componentData.push_back({ &prop, {} });
				auto& keyFrames = componentData.back().KeyFrames;

				for (const auto& keyFrame : prop.GetKeyFrames())
				{
					keyFrames.push_back(keyFrame.Frame);
				}
			}
		}
	}
}
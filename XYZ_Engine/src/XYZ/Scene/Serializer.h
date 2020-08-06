#pragma once
#include "XYZ/Core/Ref.h"


namespace XYZ {

	class Serializer
	{
	public:
		template <typename T> 
		void Serialize(const Ref<T>& item, const std::string& filepath);

		template <typename T>
		Ref<T> Deserialize(const std::string& filepath);
	};
	
}
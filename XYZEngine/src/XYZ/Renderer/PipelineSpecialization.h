#pragma once

namespace XYZ {
	struct PipelineSpecializationValue
	{
		PipelineSpecializationValue(std::string name);
			
		std::string				 Name;
		std::array<std::byte, 4> Data;
	};

	struct PipelineSpecialization
	{
		template <typename T>
		void Set(const std::string& name, T value);

		const std::vector<PipelineSpecializationValue>& GetValues() const { return m_Values; }
	private:
		std::vector<PipelineSpecializationValue> m_Values;
		mutable bool m_Dirty = false;

		friend class PipelineCache;
	};

	template<typename T>
	inline void PipelineSpecialization::Set(const std::string& name, T value)
	{
		XYZ_ASSERT(sizeof(T) <= 4, "");
		for (auto& val : m_Values)
		{
			if (val.Name == name)
			{
				memcpy(val.Data.data(), &value, sizeof(T));
				return;
			}
		}
		auto& val = m_Values.emplace_back(name);
		memcpy(val.Data.data(), &value, sizeof(T));
		m_Dirty = true;
	}
}
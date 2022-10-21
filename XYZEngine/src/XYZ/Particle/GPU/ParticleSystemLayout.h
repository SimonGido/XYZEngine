#pragma once


namespace XYZ {
	enum class ParticleVariableType
	{
		None,
		Float,
		Vec2,
		Vec3,
		Vec4,

		Int,
		IVec2,
		IVec3,
		IVec4,

		Bool
	};

	struct ParticleVariable
	{
		std::string			 Name;
		ParticleVariableType Type = ParticleVariableType::None;
		uint32_t			 Offset = 0;
		uint32_t			 Size = 0;
	};


	struct ParticleSystemLayout
	{
		using ParticleVariableInit = std::pair<std::string, ParticleVariableType>;

		ParticleSystemLayout(const std::vector<ParticleVariableInit>& particleLayout);

		uint32_t GetVariableOffset(const std::string_view name) const;

		const std::vector<ParticleVariable>& GetVariables() const { return m_Variables; }
		uint32_t							 GetStride() const;
	private:
		std::vector<ParticleVariable> m_Variables;
		uint32_t					  m_Stride;
	};
}
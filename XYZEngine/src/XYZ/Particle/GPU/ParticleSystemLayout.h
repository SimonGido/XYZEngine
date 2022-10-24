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
		bool				 IsArray = false;
		uint32_t			 Offset = 0;
		uint32_t			 Size = 0;
	
		std::string GLSLType() const;
	};


	struct ParticleSystemLayout
	{
		using ParticleVariableInit = std::pair<std::string, ParticleVariableType>;

		ParticleSystemLayout(std::string name, const std::vector<ParticleVariableInit>& particleLayout, bool round = true);

		uint32_t GetVariableOffset(const std::string_view name) const;

		const std::string& GetName() const { return m_Name; }
		const std::vector<ParticleVariable>& GetVariables() const { return m_Variables; }
		uint32_t							 GetStride() const;
	private:
		std::string					  m_Name;
		std::vector<ParticleVariable> m_Variables;
		uint32_t					  m_Stride;
	};
}
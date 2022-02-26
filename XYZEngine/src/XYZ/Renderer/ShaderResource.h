#pragma once
#include <string>
#include <vector>

namespace XYZ {

	enum class ShaderResourceType
	{
		None, Sampler2D
	};
	
	class ShaderResourceDeclaration
	{
	public:
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t resourceRegister, uint32_t count, ShaderResourceType type = ShaderResourceType::None)
			: m_Name(name), m_Register(resourceRegister), m_Count(count), m_Type(type) {}

		const std::string& GetName() const { return m_Name; }
		uint32_t		   GetRegister() const { return m_Register; }
		uint32_t		   GetCount() const { return m_Count; }
		ShaderResourceType GetType() const { return m_Type; }
	private:
		std::string		   m_Name;
		uint32_t		   m_Register = 0;
		uint32_t		   m_Count = 0;
		ShaderResourceType m_Type = ShaderResourceType::None;
	};

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;

}
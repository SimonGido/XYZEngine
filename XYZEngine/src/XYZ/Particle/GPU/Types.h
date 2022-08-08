#pragma once



namespace XYZ {
	enum class ShaderVariableType
	{
		None, Float, Vec2, Vec3, Vec4, Int, IVec2, IVec3, IVec4, UInt
	};
	
	class ShaderVariable
	{
	public:
		ShaderVariable() = default;
		ShaderVariable(std::string name, ShaderVariableType type);

		std::string ToString() const;

		std::string		   Name;
		ShaderVariableType Type = ShaderVariableType::None;
	};

	class ShaderStruct
	{
	public:
		ShaderStruct() = default;
		ShaderStruct(std::string name, const std::initializer_list<ShaderVariable>& variables);
		ShaderStruct(std::string name, std::vector<ShaderVariable> variables);

		std::string ToString() const;

		std::string					Name;
		std::vector<ShaderVariable> Variables;
	};

	class ShaderBuffer
	{
	public:


		std::string Name;
		uint32_t    Binding;
	};
}
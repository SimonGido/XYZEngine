#pragma once



namespace XYZ {

	class XYZ_API ShaderIncluder
	{
	public:
		void AddIncludes(const std::string& filepath);
		void AddIncludeFromFile(const std::string& filepath);

		const std::unordered_map<std::string, std::string>& GetIncludes() const { return m_Includes; }

	private:
		void processDirectory(const std::filesystem::path& path);

	private:
		std::unordered_map<std::string, std::string> m_Includes;
	};

}
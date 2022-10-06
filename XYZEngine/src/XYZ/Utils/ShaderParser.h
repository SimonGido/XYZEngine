#pragma once


namespace XYZ {
	class ShaderParser
	{
	public:
		struct ShaderLayoutInfo
		{
			std::string Name;
			std::string Keyword;
			uint32_t	Location;
		};

		enum ShaderStage
		{
			None,
			Vertex,
			Fragment,
			Compute
		};

		ShaderParser(std::string sourceCode);

		void AddKeyword(std::string keyword);

		void RemoveKeywordsFromSourceCode();

		std::vector<ShaderLayoutInfo> ParseLayoutInfo(ShaderStage stage) const;

		const std::string& GetSourceCode() const { return m_SourceCode; }
		const std::unordered_map<ShaderStage, std::string>& GetSources() const { return m_Sources; }
	private:
		size_t findKeywordInSourceCode(const std::string& source, size_t start, size_t end, std::string& foundWord) const;
	
		bool parseLayout(const std::string& source, size_t offset, ShaderLayoutInfo& info, size_t& end) const;

		void extractSources();
	private:
		std::string m_SourceCode;
		std::unordered_map<ShaderStage, std::string> m_Sources;
		std::vector<std::string> m_Keywords;
	};
}
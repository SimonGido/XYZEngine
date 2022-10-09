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


		ShaderParser();

		void AddKeyword(std::string keyword);

		void RemoveKeywordsFromSourceCode(std::string& source);

		std::unordered_map<std::string, std::string>	  ParseStages(const std::string& source) const;
		std::unordered_map<std::string, ShaderLayoutInfo> ParseLayoutInfo(const std::string& source) const;
		
	private:
		size_t findKeywordInSourceCode(const std::string& source, size_t start, size_t end, std::string& foundWord) const;
	
		bool parseLayout(const std::string& source, size_t offset, ShaderLayoutInfo& info, size_t& end) const;

		void extractSources(const std::string& source, std::unordered_map<std::string, std::string>& stages) const;
	private:
		std::vector<std::string> m_Keywords;
	};
}
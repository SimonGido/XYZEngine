#pragma once
#include <string>


namespace XYZ {

	class Serializable
	{
	public:
		Serializable(const std::string& filepath = "");
			
		virtual ~Serializable() {};

		void SetFilepath(const std::string& filepath);

		const std::string& GetName() const { return m_Name; }
		const std::string& GetFilepath() const { return m_Filepath; }
	protected:
		std::string m_Filepath;
		std::string m_Name;
	};
}
#pragma once
#include <string>


namespace XYZ {

	class Serializable
	{
	public:
		Serializable(const std::string& filepath = "")
			: m_Filepath(filepath)
		{}

		virtual ~Serializable() {};

		void SetFilepath(const std::string& filepath) { m_Filepath = filepath; }

		const std::string& GetFilepath() const { return m_Filepath; }

	protected:
		std::string m_Filepath;
	};
}
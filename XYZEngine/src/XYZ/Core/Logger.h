#pragma once
#include "Core.h"

#include <fstream>
#include <iostream>

namespace XYZ {

	namespace LogLevel {
		enum LogLevel
		{
			NOLOG = 1 << 0,
			INFO = 1 << 1,
			WARNING = 1 << 2,
			ERR = 1 << 3,
			API = 1 << 4,
			TRACE = API | ERR | WARNING | INFO
		};
	}
	class Logger
	{
	public:
		Logger();
		Logger(const Logger&) = delete;
		static void Init();

		template <typename... Args>
		void Info(Args... args)
		{
			if ((m_LogLevel & LogLevel::INFO))
			{
				SetColor(m_GreenColor);
				std::cout << currentDateTime() << " ";
				(std::cout << ... << args) << std::endl;
				(m_LogFile << ... << args) << std::endl;
				SetColor(m_WhiteColor);
			}
		}

		template <typename... Args>
		void Warn(Args... args)
		{
			if ((m_LogLevel & LogLevel::WARNING))
			{
				SetColor(m_YellowColor);
				std::cout << currentDateTime() << " ";
				(std::cout << ... << args) << std::endl;
				(m_LogFile << ... << args) << std::endl;
				SetColor(m_WhiteColor);
			}
		}

		template <typename... Args>
		void Error(Args... args)
		{
			if ((m_LogLevel & LogLevel::ERR))
			{
				SetColor(m_RedColor);
				std::cout << currentDateTime() << " ";
				(std::cout << ... << args) << std::endl;
				(m_LogFile << ... << args) << std::endl;
				SetColor(m_WhiteColor);
			}
		}

		template <typename... Args>
		void API(Args... args)
		{
			if ((m_LogLevel & LogLevel::API))
			{
				SetColor(m_PurpleColor);
				std::cout << currentDateTime() << " ";
				(std::cout << ... << args) << std::endl;
				(m_LogFile << ... << args) << std::endl;
				SetColor(m_WhiteColor);
			}
		}


		inline void SetLogLevel(int level) { m_LogLevel = level; };
		inline void SetLogFile(const std::string& logfile) { m_FileName = logfile; };

		static Logger& Get() { return s_Instance; };
		
	protected:
		void SetColor(const int color);

		const int m_RedColor;
		const int m_YellowColor;
		const int m_GreenColor;
		const int m_PurpleColor;
		const int m_WhiteColor;

		int m_LogLevel;
		std::ofstream m_LogFile;
		std::string m_FileName;


	private:
		std::string currentDateTime();
		static Logger s_Instance;

	};

#define XYZ_LOG_INFO(...)  Logger::Get().Info(__FUNCTION__,": ", __VA_ARGS__)
#define XYZ_LOG_WARN(...) Logger::Get().Warn(__FUNCTION__,": ",__VA_ARGS__)
#define XYZ_LOG_ERR(...)  Logger::Get().Error(__FUNCTION__,": ",__VA_ARGS__)
#define XYZ_LOG_API(...)  Logger::Get().API(__FUNCTION__,": ",__VA_ARGS__)
}
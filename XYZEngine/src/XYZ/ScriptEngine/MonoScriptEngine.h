#pragma once


namespace XYZ {


	class MonoScriptEngine
	{
	public:
		static void Init(const std::string& assemblyPath);
		static void Destroy();

	};
}
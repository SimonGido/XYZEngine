#pragma once


namespace XYZ {
	class ImGuiStyleSerializer
	{
	public:
		ImGuiStyleSerializer(bool srgb);

		void SaveStyle(const std::string& filepath);
		void LoadStyle(const std::string& filepath);

	private:
		bool m_SRGBColorSpace;
	};
}
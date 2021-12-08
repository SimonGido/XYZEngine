#pragma once


namespace XYZ {
	class ImGuiStyleSerializer
	{
	public:
		ImGuiStyleSerializer(bool srgb);

	private:
		void saveStyle(const std::string& filepath);
		void loadStyle(const std::string& filepath);

	private:
		bool m_SRGBColorSpace;

		friend class ImGuiLayer;
	};
}
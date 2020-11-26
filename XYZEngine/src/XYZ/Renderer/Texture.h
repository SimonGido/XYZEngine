#pragma once

#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/Serializable.h"

#include <string>
#include <memory>


namespace XYZ {
	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	enum class TextureParam
	{
		None = 0,
		Nearest = 1,
		Linear = 2
	};


	struct TextureSpecs
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Channels;
		TextureWrap Wrap;
		TextureParam MinParam = TextureParam::None;
		TextureParam MagParam = TextureParam::None;
	};

	/**
	* @interface Texture
	* pure virtual (interface) class.
	*/
	class Texture : public RefCount,
					public Serializable
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetChannels() const = 0;
		virtual uint32_t GetRendererID() const = 0;
 		virtual void SetData(void* data, uint32_t size) = 0;
		virtual uint8_t* GetData() = 0;
		virtual const TextureSpecs& GetSpecification() const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);
	};

	/**
	* @class Texture2D
	* @brief class Derived from Texture
	*/
	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecs& specs);
		static Ref<Texture2D> Create(TextureWrap wrap, TextureParam min, TextureParam max, const std::string& path);
		static Ref<Texture2D> Create(uint32_t rendererID);

		static void BindStatic(uint32_t rendererID, uint32_t slot);
	};

}
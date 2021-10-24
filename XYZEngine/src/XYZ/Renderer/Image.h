#pragma once

namespace XYZ {
	enum class ImageFormat
	{
		None = 0,

		// Color
		RED,
		RED32F,
		RGB,
		RGBA8,
		RGBA16F,
		RGBA32F,
		RG32F,
		R32I,

		SRGB,
		// Depth/stencil
		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};
}
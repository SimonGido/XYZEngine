#pragma once


namespace XYZ {
	/**
	* enum class represents flags for the material sort key
	*/
	enum class RenderFlags : uint64_t
	{
		MaterialFlag	= 1ULL << 0,
		TransparentFlag	= 1ULL << 16,
		InstancedFlag	= 1ULL << 17
	};

	inline RenderFlags operator | (RenderFlags lhs, RenderFlags rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return static_cast<RenderFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}

	inline RenderFlags& operator |= (RenderFlags& lhs, RenderFlags rhs)
	{
		lhs = static_cast<RenderFlags> (
			static_cast<std::underlying_type<RenderFlags>::type>(lhs) |
			static_cast<std::underlying_type<RenderFlags>::type>(rhs)
			);
		return lhs;
	}
	inline RenderFlags operator & (RenderFlags lhs, RenderFlags rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return static_cast<RenderFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
	}

	inline RenderFlags& operator &= (RenderFlags& lhs, RenderFlags rhs)
	{
		lhs = static_cast<RenderFlags> (
			static_cast<std::underlying_type<RenderFlags>::type>(lhs) &
			static_cast<std::underlying_type<RenderFlags>::type>(rhs)
			);

		return lhs;
	}

	inline RenderFlags operator | (RenderFlags lhs, int64_t rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return static_cast<RenderFlags>(static_cast<T>(lhs) | rhs);
	}

	inline RenderFlags& operator |= (RenderFlags& lhs, int64_t rhs)
	{
		lhs = static_cast<RenderFlags> (
			static_cast<std::underlying_type<RenderFlags>::type>(lhs) &
			rhs
			);

		return lhs;
	}
	inline RenderFlags operator & (RenderFlags lhs, int64_t rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return static_cast<RenderFlags>(static_cast<T>(lhs) & rhs);
	}

	inline RenderFlags& operator &= (RenderFlags& lhs, int64_t rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}



	inline int64_t operator | (int64_t lhs, RenderFlags rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return lhs | static_cast<T>(rhs);
	}

	inline int64_t operator |= (int64_t& lhs, RenderFlags rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		lhs = lhs | static_cast<T>(rhs);
		return lhs;
	}
	inline int64_t operator & (int64_t lhs, RenderFlags rhs)
	{
		using T = std::underlying_type_t <RenderFlags>;
		return lhs & static_cast<T>(rhs);
	}

	inline int64_t operator &= (int64_t& lhs, RenderFlags rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}
}
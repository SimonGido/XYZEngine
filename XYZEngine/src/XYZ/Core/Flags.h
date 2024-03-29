#pragma once
#include "Core.h"

#include <type_traits>
#include <bitset>

namespace XYZ {

	template <class T>
	using is_enum_class = std::integral_constant<bool, !std::is_convertible<T, int>{} && std::is_enum<T>{} > ;

	template<typename Enum, bool IsEnum = is_enum_class<Enum>::value>
	class Flags;

	template<typename Enum>
	class XYZ_API Flags<Enum, true>
	{
	public:
		constexpr const static int number_of_bits = std::numeric_limits<typename std::underlying_type<Enum>::type>::digits;

		constexpr Flags() = default;
		constexpr Flags(Enum value) : m_Bits(1 << static_cast<std::size_t>(value)) {}
		constexpr Flags(const Flags& other) : m_Bits(other.m_Bits) {}

		constexpr Flags operator|(Enum value) const { Flags result = *this; result.m_Bits |= 1 << static_cast<std::size_t>(value); return result; }
		constexpr Flags operator&(Enum value) const { Flags result = *this; result.m_Bits &= 1 << static_cast<std::size_t>(value); return result; }
		constexpr Flags operator^(Enum value) const { Flags result = *this; result.m_Bits ^= 1 << static_cast<std::size_t>(value); return result; }
		constexpr Flags operator~() const { Flags result = *this; result.bits.flip(); return result; }

		constexpr Flags& operator|=(Enum value) { m_Bits |= 1ull << static_cast<std::size_t>(value); return *this; }
		constexpr Flags& operator&=(Enum value) { m_Bits &= 1ull << static_cast<std::size_t>(value); return *this; }
		constexpr Flags& operator^=(Enum value) { m_Bits ^= 1ull << static_cast<std::size_t>(value); return *this; }

		constexpr bool Any() const { return m_Bits.any(); }
		constexpr bool All() const { return m_Bits.all(); }
		constexpr bool None() const { return m_Bits.none(); }
		constexpr operator bool() const { return m_Bits.any(); }

		constexpr bool Test(Enum value) const { return m_Bits.test(1ull << static_cast<std::size_t>(value)); }
		constexpr void Set(Enum value) { m_Bits.set(1ull << static_cast<std::size_t>(value)); }
		constexpr void Unset(Enum value) { m_Bits.reset(1ull << static_cast<std::size_t>(value)); }

		unsigned long ToUlong()		  const { return m_Bits.to_ulong(); }
		unsigned long long ToUllong() const { return m_Bits.to_ullong(); }
	private:
		std::bitset<number_of_bits> m_Bits;
	};

	template<typename Enum>
	constexpr typename std::enable_if_t<is_enum_class<Enum>::value, Flags<Enum>>::type operator|(Enum left, Enum right)
	{
		return Flags<Enum>(left) | right;
	}
	template<typename Enum>
	constexpr typename std::enable_if_t<is_enum_class<Enum>::value, Flags<Enum>>::type operator&(Enum left, Enum right)
	{
		return Flags<Enum>(left) & right;
	}
	template<typename Enum>
	constexpr typename std::enable_if_t<is_enum_class<Enum>::value, Flags<Enum>>::type operator^(Enum left, Enum right)
	{
		return Flags<Enum>(left) ^ right;
	}

	
}
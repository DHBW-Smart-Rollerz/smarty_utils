#ifndef UART_COMMON_HPP_
#define UART_COMMON_HPP_

#include <cstdint>
#include <span>
#include <type_traits>

#define layout_packed(Align) __attribute__((packed, aligned(Align)))

template<typename Type>
auto as(std::span<std::uint8_t> buffer) -> Type*
{
  return reinterpret_cast<Type*>(buffer.data());
}

template<typename Type>
auto as(std::span<const std::uint8_t> buffer) -> const Type*
{
  return reinterpret_cast<const Type*>(buffer.data());
}

template<typename Type, typename Underlying = std::underlying_type_t<Type>>
requires(std::is_enum_v<Type>)
constexpr auto from_underlying(const Underlying value) -> Type
{
  return static_cast<Type>(value);
}

template<typename Type, typename Underlying = std::underlying_type_t<Type>>
requires(std::is_enum_v<Type>)
constexpr auto to_underlying(const Type value) -> Underlying
{
  return static_cast<Underlying>(value);
}

#endif  // UART_COMMON_HPP_

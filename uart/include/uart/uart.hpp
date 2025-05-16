#ifndef UART_HPP_
#define UART_HPP_

#include <cstdint>
#include <cmath>

#include <utility>
#include <type_traits>
#include <array>
#include <vector>
#include <span>
#include <ranges>

#include <uart/common.hpp>

struct layout_packed(1) vec3
{
  std::float_t x;
  std::float_t y;
  std::float_t z;
}; // struct vec3

struct layout_packed(1) imu_data
{
  vec3 gyro;
  vec3 accel;
}; // struct imu_data

enum class sensor_type : std::uint8_t
{
  tof1 = 0x00,
  tof2 = 0x01,
  imu = 0x02,
  drive_mode = 0x04,
  speed_sensor = 0x05
}; // enum class sensor_type

enum class actuator_type : std::uint8_t
{
  servo = 0x03,
  motor = 0x02
}; // enum class actuator_type

template<actuator_type Type>
struct actuator_payload;

template<>
struct actuator_payload<actuator_type::servo>
{
  using type = std::int16_t;
}; // struct actuator_payload

template<>
struct actuator_payload<actuator_type::motor>
{
  using type = std::float_t;
}; // struct actuator_payload

template<actuator_type Type>
using actuator_payload_t = typename actuator_payload<Type>::type;

enum class control_character : std::uint8_t
{
  start = 0x7F,
  escape = 0x7D,
  mask = 0x20
}; // enum class control_character

template<actuator_type Type>
// requires (std::is_trivial_v<Type> && std::is_standard_layout_v<Type>)
struct layout_packed(1) package
{
  std::uint8_t start;
  std::uint8_t size;
  std::uint8_t type;
  actuator_payload_t<Type> payload;
  std::uint8_t checksum;
}; // struct package

template<actuator_type Type>
auto calculate_checksum(const package<Type> & package) -> std::uint8_t
{
  auto checksum = static_cast<std::uint8_t>(package.size);
  checksum ^= package.type;

  const auto * payload = reinterpret_cast<const std::uint8_t *>(&package.payload);

  for (auto i = 0u; i < sizeof(package.payload); ++i) {
    checksum ^= payload[i];
  }

  return checksum;
}

template<actuator_type Type>
struct package_size
{
  static constexpr auto value = sizeof(package<Type>);
}; // struct package_size

template<actuator_type Type>
constexpr auto package_size_v = package_size<Type>::value;

template<actuator_type Type>
struct payload_size
{
  // exclude start, size and checksum
  static constexpr auto value = package_size_v<Type>-3u;
}; // struct package_size

template<actuator_type Type>
constexpr auto payload_size_v = payload_size<Type>::value;

template<actuator_type Type>
auto serialize_package(const actuator_payload_t<Type> & value) -> std::array<std::uint8_t,
  package_size_v<Type>>
{
  auto buffer = std::array<std::uint8_t, package_size_v<Type>>{};

  auto  p = reinterpret_cast<package<Type> *>(buffer.data());

  p->start = to_underlying(control_character::start);
  p->size = payload_size_v<Type>;
  p->type = to_underlying(Type);
  p->payload = value;
  p->checksum = calculate_checksum(*p);

  return buffer;
}

auto encode_buffer(std::span<const std::uint8_t> source) -> std::vector<std::uint8_t>
{
  auto destination = std::vector<std::uint8_t>{};
  destination.reserve(source.size() * 2u); // worst case scenario

  destination.push_back(source[0]);

  for (const auto byte : source.subspan(1)) {
    switch (byte) {
      case to_underlying(control_character::start):
      case to_underlying(control_character::escape): {
          destination.push_back(to_underlying(control_character::escape));
          destination.push_back(byte ^ to_underlying(control_character::mask));
          break;
        }
      default: {
          destination.push_back(byte);
          break;
        }
    }
  }

  return destination;
}

#endif // UART_HPP_

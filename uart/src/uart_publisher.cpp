#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <span>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <geometry_msgs/msg/vector3.hpp>

#include <boost/asio.hpp>

#define layout_packed(Align) __attribute__((packed, aligned(Align)))

// usbipd list
// usbipd attach --wsl --busid 2-9
// colcon build && source install/setup.sh && ros2 run uart uart_publisher
// ls /dev/ttyUSB* 2>/dev/null

struct layout_packed(1) vec3 {
  std::float_t x;
  std::float_t y;
  std::float_t z;
}; // struct vec3

enum class sensor_type : std::uint8_t {
  tof1 = 0x00,
  tof2 = 0x01,
  imu_gyro = 0x02,
  imu_accel = 0x03
}; // enum class sensor_type

template<typename Type>
auto as(std::span<std::uint8_t> buffer) -> Type* {
  return reinterpret_cast<Type*>(buffer.data());
}

template<typename Type>
auto as(std::span<const std::uint8_t> buffer) -> const Type* {
  return reinterpret_cast<const Type*>(buffer.data());
}

template<typename Type, typename Underlying = std::underlying_type_t<Type>>
requires (std::is_enum_v<Type>)
auto from_underlying(const Underlying value) -> Type {
  return static_cast<Type>(value);
}

template<typename Type, typename Underlying = std::underlying_type_t<Type>>
requires (std::is_enum_v<Type>)
auto to_underlying(const Type value) -> Underlying {
  return static_cast<Underlying>(value);
}

class uart_publisher : public rclcpp::Node {

  using base = rclcpp::Node;

  inline static constexpr auto start_byte = std::uint8_t{0x7F};
  inline static constexpr auto escape_byte = std::uint8_t{0x7D};
  inline static constexpr auto escape_mask = std::uint8_t{0x20};

  inline static constexpr auto rx_buffer_size = std::uint8_t{64u};

  public:

  uart_publisher()
  : base{"uart_publisher"},
    _io_context{},
    _serial_port{_io_context},
    _state{read_state::wait_start},
    _package_index{0u},
    _needs_escaping{false},
    _expected_size{0u} {
    RCLCPP_INFO(base::get_logger(), "uart_publisher created");

    _tof1_publisher = base::create_publisher<std_msgs::msg::UInt16>("/tof1", 10);
    _tof2_publisher = base::create_publisher<std_msgs::msg::UInt16>("/tof2", 10);
    _gyro_publisher = base::create_publisher<geometry_msgs::msg::Vector3>("/gyro", 10);
    _accel_publisher = base::create_publisher<geometry_msgs::msg::Vector3>("/accel", 10);

    _serial_port.open("/dev/ttyUSB0");

    _serial_port.set_option(boost::asio::serial_port::baud_rate(115200));
    _serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    _serial_port.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
    _serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    _serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));

    while (true) {
      _update();
    }
  }

private:

  enum class read_state : std::uint8_t {
    wait_start,
    read_size,
    read_payload,
    verify_checksum
  }; // struct read_state

  auto _update() -> void {
    const auto bytes_read = boost::asio::read(_serial_port, boost::asio::buffer(_rx_buffer.data(), _rx_buffer.size()));

    for (auto i = 0u; i < bytes_read; ++i) {
      _current_byte = _rx_buffer[i];

      if (!_decode()) {
        continue;
      }

      switch (_state) {
        case read_state::wait_start: {
          if (_current_byte == start_byte) {
            _package_index = 0u;
            _state = read_state::read_size;
          } else {
            RCLCPP_WARN(base::get_logger(), "Unexpected byte: 0x%02X", _current_byte);
          }

          break;
        }
        case read_state::read_size: {
          if (_current_byte == start_byte) {
            RCLCPP_WARN(base::get_logger(), "Unexpected start byte 0x7E in read_size state");
            _state = read_state::wait_start;

            break;
          }

          _expected_size = _current_byte;
          
          if (_expected_size > _rx_buffer.size() - 1u) {
            RCLCPP_WARN(base::get_logger(), "Invalid size byte 0x%02X exceeds buffer size", _current_byte);
            _state = read_state::wait_start;
          } else {
            _state = read_state::read_payload;
          }

          break;
        }
        case read_state::read_payload: {
          if (_current_byte == start_byte) {
            RCLCPP_WARN(base::get_logger(), "Unexpected start byte 0x7F in payload");
            _state = read_state::wait_start;
            break;
          }

          if (!_decode()) {
            continue;
          }

          _package_buffer[_package_index++] = _current_byte;

          if (_package_index >= _expected_size) { 
            _state = read_state::verify_checksum;
          }
        
          break;
        }
        case read_state::verify_checksum: {
          if (_current_byte == start_byte) {
            RCLCPP_WARN(base::get_logger(), "Unexpected start byte 0x7F in checksum");
            _state = read_state::wait_start;
            break;
          }

          if (_verify_checksum({_package_buffer.data(), _package_index}, _current_byte)) {
            _process_function({_package_buffer.data(), _expected_size});
          } else {
            RCLCPP_WARN(base::get_logger(), "Invalid checksum");
          }

          _state = read_state::wait_start;
          break;
        }
        default: {
          RCLCPP_WARN(base::get_logger(), "Invalid state");
          _state = read_state::wait_start;
          break;
        }
      }
    }

    if (bytes_read == 0u && _state != read_state::wait_start) {
      _state = read_state::wait_start;
      RCLCPP_WARN(base::get_logger(), "Timeout: Incomplete package");
    }
  }

  auto _process_function(std::span<const std::uint8_t> buffer) -> void {
    const auto type = from_underlying<sensor_type>(buffer[0]);

    switch (type) {
      case sensor_type::tof1: {
        const auto* tof1 = as<std::uint16_t>(buffer.subspan(1));
        RCLCPP_INFO(base::get_logger(), "Received type 0x00, tof1: %u", *tof1);
        auto msg = std_msgs::msg::UInt16{};
        msg.data = *tof1;
        _tof1_publisher->publish(msg);
        break;
      }
      case sensor_type::tof2: {
        const auto* tof2 = as<std::uint16_t>(buffer.subspan(1));
        RCLCPP_INFO(base::get_logger(), "Received type 0x01, tof2: %u", *tof2);
        auto msg = std_msgs::msg::UInt16{};
        msg.data = *tof2;
        _tof2_publisher->publish(msg);
        break;
      }
      case sensor_type::imu_gyro: {
        const auto* gyro = as<vec3>(buffer.subspan(1));
        RCLCPP_INFO(base::get_logger(), "Received type 0x02, vec3: (%f, %f, %f)", gyro->x, gyro->y, gyro->z);
        auto msg = geometry_msgs::msg::Vector3{};
        msg.x = gyro->x;
        msg.y = gyro->y;
        msg.z = gyro->z;
        _gyro_publisher->publish(msg);
        break;
      }
      case sensor_type::imu_accel: {
        const auto* accel = as<vec3>(buffer.subspan(1));
        RCLCPP_INFO(base::get_logger(), "Received type 0x03, vec3: (%f, %f, %f)", accel->x, accel->y, accel->z);
        auto msg = geometry_msgs::msg::Vector3{};
        msg.x = accel->x;
        msg.y = accel->y;
        msg.z = accel->z;
        _accel_publisher->publish(msg);
        break;
      }
      default: {
        RCLCPP_WARN(base::get_logger(), "Unknown type 0x%02X", to_underlying(type));
        break;
      }
    }
  }

  auto _verify_checksum(std::span<const std::uint8_t> buffer, std::uint8_t checksum) -> bool {
    // Size is part of the packages checksum
    auto calculated_checksum = static_cast<std::uint8_t>(buffer.size());

    for (const auto byte : buffer) {
      calculated_checksum ^= byte;
    }

    return calculated_checksum == checksum;
  }

  auto _decode() -> bool {
    if (_current_byte == escape_byte) {
      _needs_escaping = true;
      return false;
    }

    if (_needs_escaping) {
      _current_byte ^= escape_mask;
      _needs_escaping = false;
    }

    return true;
  }

  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _tof1_publisher;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _tof2_publisher;
  rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr _gyro_publisher;
  rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr _accel_publisher;

  boost::asio::io_context _io_context;
  boost::asio::serial_port _serial_port;
  
  read_state _state;
  std::array<std::uint8_t, rx_buffer_size> _rx_buffer;
  std::array<std::uint8_t, 32u> _package_buffer;
  std::size_t _package_index;
  std::uint8_t _current_byte;
  bool _needs_escaping;
  std::uint8_t _expected_size;

}; // class uart_publisher

auto main(int argc, char** argv) -> int {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<uart_publisher>());
  rclcpp::shutdown();
  return 0;
}

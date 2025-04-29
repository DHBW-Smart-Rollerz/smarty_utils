#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <span>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include <boost/asio.hpp>

#include <uart/uart.hpp>

// usbipd list
// usbipd attach --wsl --busid 2-9
// colcon build && source install/setup.sh && ros2 run uart uart_publisher
// ls /dev/ttyUSB* 2>/dev/null

class uart_publisher : public rclcpp::Node {

  using base = rclcpp::Node;

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

    _tof1_publisher = base::create_publisher<std_msgs::msg::UInt16>("/sensor/tof/front_left", 10);
    _tof2_publisher = base::create_publisher<std_msgs::msg::UInt16>("/sensor/tof/front_right", 10);
    _imu_publisher = base::create_publisher<sensor_msgs::msg::Imu>("/sensor/imu", 10);
    _drive_mode_publisher = base::create_publisher<std_msgs::msg::UInt8>("/remote/drive_mode", 10);

    _serial_port.open("/dev/ttyUSB0");

    _serial_port.set_option(boost::asio::serial_port::baud_rate(115200));
    _serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    _serial_port.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
    _serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    _serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));

    RCLCPP_INFO(base::get_logger(), "uart_publisher created");

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

      switch (_state) {
        case read_state::wait_start: {
          if (_current_byte == to_underlying(control_character::start)) {
            _package_index = 0u;
            _state = read_state::read_size;
          } else {
            RCLCPP_WARN(base::get_logger(), "Unexpected byte: 0x%02X", _current_byte);
          }

          break;
        }
        case read_state::read_size: {
          // if (_current_byte == to_underlying(control_character::start)) {
          //   RCLCPP_WARN(base::get_logger(), "Unexpected start byte 0x7E in read_size state");
          //   _state = read_state::wait_start;

          //   break;
          // }

          if (!_decode()) {
            continue;
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
          if (_current_byte == to_underlying(control_character::start)) {
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
          // if (_current_byte == to_underlying(control_character::start)) {
          //   RCLCPP_WARN(base::get_logger(), "Unexpected start byte 0x7F in checksum");
          //   _state = read_state::wait_start;
          //   break;
          // }

          if (!_decode()) {
            continue;
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
      case sensor_type::drive_mode: {
        const auto* drive_mode = as<std::uint8_t>(buffer.subspan(1));
        // RCLCPP_INFO(base::get_logger(), "Received type 0x02, vec3: (%f, %f, %f)", gyro->x, gyro->y, gyro->z);
        auto msg = std_msgs::msg::UInt8{};
        msg.data = *drive_mode;
        _drive_mode_publisher->publish(msg);
        break;
      }
      case sensor_type::imu: {
        const auto* imu = as<imu_data>(buffer.subspan(1));
        // RCLCPP_INFO(base::get_logger(), "Received type 0x03, vec3: (%f, %f, %f)", accel->x, accel->y, accel->z);
        auto msg = sensor_msgs::msg::Imu{};
        msg.angular_velocity.x = imu->gyro.x;
        msg.angular_velocity.y = imu->gyro.y;
        msg.angular_velocity.z = imu->gyro.z;
        msg.linear_acceleration.x = imu->accel.x;
        msg.linear_acceleration.y = imu->accel.y;
        msg.linear_acceleration.z = imu->accel.z;
        _imu_publisher->publish(msg);
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
    if (_current_byte == to_underlying(control_character::escape)) {
      _needs_escaping = true;
      return false;
    }

    if (_needs_escaping) {
      _current_byte ^= to_underlying(control_character::mask);
      _needs_escaping = false;
    }

    return true;
  }

  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _tof1_publisher;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr _tof2_publisher;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr _imu_publisher;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr _drive_mode_publisher;

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

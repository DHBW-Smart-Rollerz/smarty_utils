#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <span>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <boost/asio.hpp>

#define layout_packed(Align) __attribute__((packed, aligned(Align)))

struct layout_packed(1) test_payload{
  std::uint32_t value;
}; // struct test_payload

struct layout_packed(1) package {
  std::uint8_t start;
  std::uint8_t size;
  std::uint8_t type;
  union layout_packed(1) {
    test_payload test;
  } payload;
  std::uint8_t checksum;
}; // struct package
  
inline static constexpr auto start_byte = std::uint8_t{0x7F};
inline static constexpr auto escape_byte = std::uint8_t{0x7D};
inline static constexpr auto escape_mask = std::uint8_t{0x20};

class uart_publisher : public rclcpp::Node {

  using base = rclcpp::Node;

public:

  uart_publisher()
  : base{"uart_publisher"},
    _io_context{},
    _serial_port{_io_context} {
    RCLCPP_INFO(base::get_logger(), "uart_publisher created");

    _publisher = base::create_publisher<std_msgs::msg::String>("/uart/test_value", 10u);

    _serial_port.open("/dev/ttyUSB0");

    _serial_port.set_option(boost::asio::serial_port::baud_rate(115200));
    _serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    _serial_port.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
    _serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    _serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));

    // boost::asio::async_read_until(_serial_port, boost::asio::dynamic_buffer(_read_buffer), 0x7D, [this](const boost::system::error_code& error_code, const std::size_t bytes_read) {
    //   RCLCPP_INFO(base::get_logger(), "async_read_until uart");
    //   _read_uart(error_code, bytes_read);
    // });

    // _io_context.run();

    auto c = char{};

    while (true) {
      boost::asio::read(_serial_port, boost::asio::buffer(&c, 1));
      RCLCPP_INFO(base::get_logger(), "Byte: 0x%02X", c);
    }
  }

private:

  auto _echo(const boost::system::error_code& error_code, const std::size_t bytes_read) -> void {

  }

  auto _read_uart(const boost::system::error_code& error_code, const std::size_t bytes_read) -> void {
    if (error_code) {
      RCLCPP_ERROR(base::get_logger(), "Error reading from UART: %s", error_code.message().c_str());
      return;
    }

    if (bytes_read != 10u) {
      RCLCPP_ERROR(base::get_logger(), "Error reading from UART: %lu bytes read", bytes_read);
      for (auto i = 0u; i < bytes_read; ++i) {
        RCLCPP_INFO(base::get_logger(), "Byte %u: %02X", i, _read_buffer[i]);
      }
    }

    _read_buffer.clear();

    boost::asio::async_read_until(_serial_port, boost::asio::dynamic_buffer(_read_buffer), 0x7D, [this](const boost::system::error_code& error_code, const std::size_t bytes_read) {
      _read_uart(error_code, bytes_read);
    });
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;

  boost::asio::io_context _io_context;
  boost::asio::serial_port _serial_port;
  std::vector<std::uint8_t> _read_buffer;

}; // class uart_publisher

auto main(int argc, char** argv) -> int {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<uart_publisher>());
  rclcpp::shutdown();
  return 0;
}

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <boost/asio.hpp>

#define layout_packed(Align) __attribute__((packed, aligned(Align)))

struct layout_packed(1) test_payload{
  std::uint32_t value;
}; // struct test_payload

template<typename Payload>
struct layout_packed(1) package {
  std::uint8_t start;
  std::uint8_t size;
  std::uint8_t type;
  Payload payload;
  std::uint8_t checksum;
}; // struct package

class uart_subscriber : public rclcpp::Node {

  using base = rclcpp::Node;

  inline static constexpr auto start_byte = std::uint8_t{0x7F};
  inline static constexpr auto escape_byte = std::uint8_t{0x7D};
  inline static constexpr auto escape_mask = std::uint8_t{0x20};

public:

  uart_subscriber()
  : base("minimal_subscriber"),
    _io_context{},
    _serial_port{_io_context} {

      
    _serial_port.open("/dev/ttyUSB0");

    _serial_port.set_option(boost::asio::serial_port::baud_rate(115200));
    _serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    _serial_port.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
    _serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    _serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));

    _subscription = base::create_subscription<std_msgs::msg::String>("topic", 10, std::bind(&uart_subscriber::_callback, this, std::placeholders::_1));

    _io_context.run();

    RCLCPP_INFO(base::get_logger(), "uart_subscriber created");
  }

private:

  template<typename Payload>
  auto _create_package(std::uint8_t* buffer, const Payload& payload, const std::uint8_t type) -> void {
    auto* p = reinterpret_cast<package<Payload>*>(buffer);

    p->start = start_byte;
    p->size = sizeof(Payload) + 1u;
    p->type = type;
    p->payload = payload;
    p->checksum = _calculate_checksum(buffer + 1, sizeof(Payload) + 2u);
  }

  auto _callback(std_msgs::msg::String::UniquePtr msg) -> void {
    RCLCPP_INFO(base::get_logger(), "Callback %s", msg->data.c_str());

    auto buffer = std::array<std::uint8_t, sizeof(package<test_payload>)>{};

    _create_package(buffer.data(), test_payload{12345}, 0x01);
    
    _serial_port.write_some(boost::asio::buffer(buffer.data(), buffer.size()));
  }

  auto _calculate_checksum(const std::uint8_t* buffer, const std::size_t size) -> std::uint8_t {
    auto checksum = buffer[0];

    for (auto i = 1u; i < size; ++i) {
      checksum ^= buffer[i];
    }

    return checksum;
  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr _subscription;

  boost::asio::io_context _io_context;
  boost::asio::serial_port _serial_port;

}; // class uart_subscriber

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<uart_subscriber>());
  rclcpp::shutdown();
  return 0;
}

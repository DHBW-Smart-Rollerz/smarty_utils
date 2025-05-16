#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/u_int32.hpp>
#include <std_msgs/msg/int16.hpp>
#include <std_msgs/msg/float32.hpp>

#include <boost/asio.hpp>

#include <uart/uart.hpp>

class uart_subscriber : public rclcpp::Node {

  using base = rclcpp::Node;

public:
  uart_subscriber()
  : base("minimal_subscriber"),
    _io_context{},
    _serial_port{_io_context}
  {

    _serial_port.open("/dev/ttyUSB0");

    _serial_port.set_option(boost::asio::serial_port::baud_rate(115200));
    _serial_port.set_option(boost::asio::serial_port::flow_control(
      boost::asio::serial_port::flow_control::none));
    _serial_port.set_option(boost::asio::serial_port::character_size(
      boost::asio::serial_port::character_size(8)));
    _serial_port.set_option(boost::asio::serial_port::parity(
      boost::asio::serial_port::parity::none));
    _serial_port.set_option(boost::asio::serial_port::stop_bits(
      boost::asio::serial_port::stop_bits::one));

    _servo_subscription = base::create_subscription<std_msgs::msg::Int16>("/servo", 10,
      std::bind(&uart_subscriber::_servo_callback, this, std::placeholders::_1));
    _motor_subscription = base::create_subscription<std_msgs::msg::Float32>("/motor", 10,
      std::bind(&uart_subscriber::_motor_callback, this, std::placeholders::_1));

    // _io_context.run();

    RCLCPP_INFO(base::get_logger(), "uart_subscriber created");
  }

private:
  auto _servo_callback(std_msgs::msg::Int16::UniquePtr msg) -> void
  {
    RCLCPP_INFO(base::get_logger(), "servo_callback %d", msg->data);

    const auto package = serialize_package<actuator_type::servo>(msg->data);
    const auto encoded = encode_buffer(package);

    // _serial_port.write_some(boost::asio::buffer(encoded.data(), encoded.size()));
    boost::asio::write(_serial_port, boost::asio::buffer(encoded.data(), encoded.size()));
  }

  auto _motor_callback(std_msgs::msg::Float32::UniquePtr msg) -> void
  {
    RCLCPP_INFO(base::get_logger(), "motor_callback %f", msg->data);

    const auto package = serialize_package<actuator_type::motor>(msg->data);
    const auto encoded = encode_buffer(package);

    // _serial_port.write_some(boost::asio::buffer(buffer.data(), buffer.size()));
    boost::asio::write(_serial_port, boost::asio::buffer(encoded.data(), encoded.size()));
  }

  rclcpp::Subscription<std_msgs::msg::Int16>::SharedPtr _servo_subscription;
  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr _motor_subscription;

  boost::asio::io_context _io_context;
  boost::asio::serial_port _serial_port;

}; // class uart_subscriber

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<uart_subscriber>());
  rclcpp::shutdown();
  return 0;
}

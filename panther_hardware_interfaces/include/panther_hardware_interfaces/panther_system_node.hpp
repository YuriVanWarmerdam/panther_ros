#ifndef PANTHER_HARDWARE_INTERFACES__PANTHER_SYSTEM_NODE_HPP_
#define PANTHER_HARDWARE_INTERFACES__PANTHER_SYSTEM_NODE_HPP_

#include <memory>
#include <thread>

#include <rclcpp/rclcpp.hpp>

#include <realtime_tools/realtime_publisher.h>

#include <std_srvs/srv/trigger.hpp>

#include <panther_msgs/msg/driver_state.hpp>

#include <panther_hardware_interfaces/roboteq_data_converters.hpp>

namespace panther_hardware_interfaces
{

// TODO Rename class
/**
 * @brief Class that takes care of additional ROS interface of panther system, such as publishing 
 * driver state and providing service for clearing errors
 */
class PantherSystemNode
{
public:
  PantherSystemNode() {}

  /**
   * @brief Creates node and executor (in a separate thread)
   */
  void Configure();

  /**
   * @brief Creates publishers, subscribers and services
   * @param clear_errors - functions that should be called, when clear errors
   * service is called
   */
  void Activate(std::function<void()> clear_errors);

  /**
   * @brief Destroys publishers, subscribers and services
   */
  void Deactivate();

  /**
   * @brief Stops executor thread and destroys the node
   */
  void Deinitialize();

  /**
   * @brief Updates fault flags, script flags, and runtime errors in the driver state msg
   */
  void UpdateMsgErrorFlags(const RoboteqData & front, const RoboteqData & rear);

  /**
   * @brief Updates parameters of the drivers: voltage, current and temperature
   */
  void UpdateMsgDriversParameters(const DriverState & front, const DriverState & rear);

  /**
   * @brief Updates current state of communication errors and general error state
   */
  void UpdateMsgErrors(
    bool is_error, bool is_write_sdo_error, bool is_read_sdo_error, bool is_read_pdo_error,
    bool front_old_data, bool rear_old_data);

  void PublishDriverState();

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::executors::SingleThreadedExecutor::UniquePtr executor_;
  std::unique_ptr<std::thread> executor_thread_;

  std::atomic_bool stop_executor_ = false;

  rclcpp::Publisher<panther_msgs::msg::DriverState>::SharedPtr driver_state_publisher_;
  std::unique_ptr<realtime_tools::RealtimePublisher<panther_msgs::msg::DriverState>>
    realtime_driver_state_publisher_;

  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr clear_errors_srv_;

  std::function<void()> clear_errors_;

  void ClearErrorsCb(
    std_srvs::srv::Trigger::Request::ConstSharedPtr /* request */,
    std_srvs::srv::Trigger::Response::SharedPtr response);
};

}  // namespace panther_hardware_interfaces

#endif  // PANTHER_HARDWARE_INTERFACES__PANTHER_SYSTEM_NODE_HPP_
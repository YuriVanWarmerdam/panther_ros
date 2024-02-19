// Copyright 2024 Husarion sp. z o.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>
#include <string>

#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>

#include <panther_manager/plugins/call_set_led_animation_service_node.hpp>

#include <panther_manager_plugin_test_utils.hpp>

void ServiceFailedCallback(const panther_msgs::srv::SetLEDAnimation::Request::SharedPtr request,
                           panther_msgs::srv::SetLEDAnimation::Response::SharedPtr response)
{
  response->message = "Failed callback pass!";
  response->success = false;
  RCLCPP_INFO_STREAM(rclcpp::get_logger("test_set_led_animation_plugin"),
                     response->message << " success: " << response->success << " id: " << request->animation.id
                                       << " param: " << request->animation.param
                                       << " repeating: " << request->repeating);
}

void ServiceSuccessCallbackCheckRepeatingTrueValue(const panther_msgs::srv::SetLEDAnimation::Request::SharedPtr request,
                                          panther_msgs::srv::SetLEDAnimation::Response::SharedPtr response)
{
  response->message = "Successfully callback pass!";
  response->success = true;
  RCLCPP_INFO_STREAM(rclcpp::get_logger("test_set_led_animation_plugin"),
                     response->message << " success: " << response->success << " id: " << request->animation.id
                                       << " param: " << request->animation.param
                                       << " repeating: " << request->repeating);

  EXPECT_EQ(request->repeating, true);
}

void ServiceSuccessCallbackCheckRepeatingFalseValue(const panther_msgs::srv::SetLEDAnimation::Request::SharedPtr request,
                                           panther_msgs::srv::SetLEDAnimation::Response::SharedPtr response)
{
  response->message = "Successfully callback pass!";
  response->success = true;
  RCLCPP_INFO_STREAM(rclcpp::get_logger("test_set_led_animation_plugin"),
                     response->message << " success: " << response->success << " id: " << request->animation.id
                                       << " param: " << request->animation.param
                                       << " repeating: " << request->repeating);

  EXPECT_EQ(request->repeating, false);
}

void ServiceSuccessCallbackCheckId5(const panther_msgs::srv::SetLEDAnimation::Request::SharedPtr request,
                                           panther_msgs::srv::SetLEDAnimation::Response::SharedPtr response)
{
  response->message = "Successfully callback pass!";
  response->success = true;
  RCLCPP_INFO_STREAM(rclcpp::get_logger("test_set_led_animation_plugin"),
                     response->message << " success: " << response->success << " id: " << request->animation.id
                                       << " param: " << request->animation.param
                                       << " repeating: " << request->repeating);

  EXPECT_EQ(request->animation.id, 5u);
}

TEST(TestCallSetLedAnimationService, good_loading_call_set_led_animation_service_plugin)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "true" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  ASSERT_NO_THROW({ test_utils.CreateTree("CallSetLedAnimationService", services); });
  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, wrong_plugin_name_loading_call_set_led_animation_service_plugin)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "true" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  EXPECT_THROW({ test_utils.CreateTree("WrongCallSetLedAnimationService", services); }, BT::RuntimeError);
  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, wrong_call_set_led_animation_service_service_server_not_initialized)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "true" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::FAILURE)
  {
    FAIL() << "Found set_led_animation service but shouldn't!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, good_set_led_animation_call_service_success_with_true_repeating_value)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "true" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  test_utils.CreateSetLEDAnimationServiceServer(ServiceSuccessCallbackCheckRepeatingTrueValue);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::SUCCESS)
  {
    FAIL() << "Cannot call set_led_animation service!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, good_set_led_animation_call_service_success_with_false_repeating_value)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "false" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  test_utils.CreateSetLEDAnimationServiceServer(ServiceSuccessCallbackCheckRepeatingFalseValue);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::SUCCESS)
  {
    FAIL() << "Cannot call set_led_animation service!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, good_set_led_animation_call_service_success_with_5_id_value)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "5", "", "false" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  test_utils.CreateSetLEDAnimationServiceServer(ServiceSuccessCallbackCheckId5);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::SUCCESS)
  {
    FAIL() << "Cannot call set_led_animation service!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, wrong_set_led_animation_call_service_failure)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "true" } } };

  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  test_utils.CreateSetLEDAnimationServiceServer(ServiceFailedCallback);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::FAILURE)
  {
    FAIL() << "Cannot call set_led_animation service!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, wrong_repeating_service_value_defined)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "0", "", "wrong_bool" } } };
  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::FAILURE)
  {
    FAIL() << "Wrong value is parsed as good value!";
  }

  test_utils.Stop();
}

TEST(TestCallSetLedAnimationService, wrong_id_service_value_defined)
{
  std::map<std::string, panther_manager_plugin_test::SetLEDAnimationTestUtils> services = { { "set_led_animation",
                                                                                              { "-5", "", "true" } } };
  panther_manager_plugin_test::PantherManagerPluginTestUtils test_utils;
  test_utils.Start();
  auto& tree = test_utils.CreateTree("CallSetLedAnimationService", services);

  auto status = tree.tickWhileRunning(std::chrono::milliseconds(100));
  if (status != BT::NodeStatus::FAILURE)
  {
    FAIL() << "Wrong value is parsed as good value!";
  }

  test_utils.Stop();
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

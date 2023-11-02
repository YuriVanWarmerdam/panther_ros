#include <string>

#include <gtest/gtest.h>

#include <mock_roboteq.hpp>
#include <panther_hardware_interfaces/canopen_controller.hpp>

#include <iostream>

class TestCanOpenController : public ::testing::Test
{
public:
  std::unique_ptr<RoboteqMock> roboteq_mock_;
  panther_hardware_interfaces::CanOpenSettings canopen_settings_;

  std::unique_ptr<panther_hardware_interfaces::CanOpenController> canopen_controller_;

  TestCanOpenController()
  {
    canopen_settings_.master_can_id = 3;
    canopen_settings_.front_driver_can_id = 1;
    canopen_settings_.rear_driver_can_id = 2;
    canopen_settings_.pdo_feedback_timeout = std::chrono::milliseconds(15);
    canopen_settings_.sdo_operation_timeout = std::chrono::milliseconds(4);

    canopen_controller_ =
      std::make_unique<panther_hardware_interfaces::CanOpenController>(canopen_settings_);

    roboteq_mock_ = std::make_unique<RoboteqMock>();
    roboteq_mock_->Start();
  }

  ~TestCanOpenController()
  {
    roboteq_mock_->Stop();
    roboteq_mock_.reset();
  }
};

TEST_F(TestCanOpenController, test_canopen_controller)
{
  ASSERT_NO_THROW(canopen_controller_->Initialize());
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());

  // Check if deinitialization worked correctly - initialize once again
  ASSERT_NO_THROW(canopen_controller_->Initialize());
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());
}

TEST_F(TestCanOpenController, test_canopen_controller_error_device_type)
{
  roboteq_mock_->front_driver_->SetOnReadWait<uint32_t>(0x1000, 0, 100000);
  ASSERT_THROW(canopen_controller_->Initialize(), std::runtime_error);
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());

  roboteq_mock_->front_driver_->SetOnReadWait<uint32_t>(0x1000, 0, 0);
  ASSERT_NO_THROW(canopen_controller_->Initialize());
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());
}

TEST_F(TestCanOpenController, test_canopen_controller_error_vendor_id)
{
  roboteq_mock_->rear_driver_->SetOnReadWait<uint32_t>(0x1018, 1, 100000);
  ASSERT_THROW(canopen_controller_->Initialize(), std::runtime_error);
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());

  roboteq_mock_->rear_driver_->SetOnReadWait<uint32_t>(0x1018, 1, 0);
  ASSERT_NO_THROW(canopen_controller_->Initialize());
  ASSERT_NO_THROW(canopen_controller_->Deinitialize());
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
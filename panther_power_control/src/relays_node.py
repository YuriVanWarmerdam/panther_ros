#!/usr/bin/python3

from dataclasses import dataclass
import RPi.GPIO as GPIO
from threading import Lock

import rospy

from geometry_msgs.msg import Twist
from std_msgs.msg import Bool
from std_srvs.srv import Trigger, TriggerRequest, TriggerResponse

from panther_msgs.msg import DriverState, IOState


@dataclass
class PatherGPIO:
    MOTOR_ON = 6  # Pin to enable motor controllers
    STAGE2_INPUT = 22  # Check if power can be forwarded to motor controllers

    def __setattr__(self, name: str, value: int) -> None:
        raise AttributeError(f'Can\'t reassign constant {name}')


class RelaysNode:
    def __init__(self, name: str) -> None:
        rospy.init_node(name, anonymous=False)

        self._lock = Lock()

        self._pins = PatherGPIO()
        self._setup_gpio()

        self._e_stop_state = not GPIO.input(self._pins.STAGE2_INPUT)
        self._cmd_vel_msg_time = rospy.get_time()
        self._can_net_err = True

        # -------------------------------
        #   Publishers
        # -------------------------------

        self._e_stop_state_pub = rospy.Publisher('hardware/e_stop', Bool, queue_size=1, latch=True)
        self._io_state_pub = rospy.Publisher('hardware/io_state', IOState, queue_size=1, latch=True)

        # init e-stop state
        self._e_stop_state_pub.publish(self._e_stop_state)

        self._io_state = IOState()
        self._io_state.motor_on = GPIO.input(self._pins.STAGE2_INPUT)
        self._io_state.aux_power = False
        self._io_state.charger_connected = False
        self._io_state.fan = False
        self._io_state.power_button = False
        self._io_state.digital_power = True
        self._io_state.charger_enabled = False
        self._io_state_pub.publish(self._io_state)

        # -------------------------------
        #   Subscribers
        # -------------------------------

        self._cmd_vel_sub = rospy.Subscriber('/cmd_vel', Twist, self._cmd_vel_cb, queue_size=1)
        self._motor_controllers_state_sub = rospy.Subscriber(
            'driver/motor_controllers_state', DriverState, self._motor_controllers_state_cb
        )

        # -------------------------------
        #   Service servers
        # -------------------------------

        self._e_stop_reset_server = rospy.Service(
            'hardware/e_stop_reset', Trigger, self._e_stop_reset_cb
        )
        self._e_stop_trigger_server = rospy.Service(
            'hardware/e_stop_trigger', Trigger, self._e_stop_trigger_cb
        )

        # -------------------------------
        #   Timers
        # -------------------------------

        # check motor state at 10 Hz
        self._set_motor_state_timer = rospy.Timer(
            rospy.Duration(0.1), self._set_motor_state_timer_cb
        )

        rospy.loginfo(f'[{rospy.get_name()}] Node started')

    def _cmd_vel_cb(self, *args) -> None:
        with self._lock:
            self._cmd_vel_msg_time = rospy.get_time()

    def _motor_controllers_state_cb(self, msg: DriverState) -> None:
        with self._lock:
            self._can_net_err = any(
                {msg.rear.fault_flag.can_net_err, msg.front.fault_flag.can_net_err}
            )

    def _e_stop_reset_cb(self, req: TriggerRequest) -> TriggerResponse:
        with self._lock:
            if rospy.get_time() - self._cmd_vel_msg_time <= 2.0:
                return TriggerResponse(
                    False,
                    'E-STOP reset failed, messages are still published on /cmd_vel topic!',
                )
            elif self._can_net_err:
                return TriggerResponse(
                    False,
                    'E-STOP reset failed, unable to communicate with motor controllers! Please check connection with motor controllers.',
                )

            self._e_stop_state = False
            self._e_stop_state_pub.publish(self._e_stop_state)
            return TriggerResponse(True, 'E-STOP reset successful')

    def _e_stop_trigger_cb(self, req: TriggerRequest) -> TriggerResponse:
        with self._lock:
            self._e_stop_state = True
            self._e_stop_state_pub.publish(self._e_stop_state)
            return TriggerResponse(True, 'E-SROP triggered successful')

    def _set_motor_state_timer_cb(self, *args) -> None:
        motor_state = GPIO.input(self._pins.STAGE2_INPUT)
        GPIO.output(self._pins.MOTOR_ON, motor_state)
        if self._io_state.motor_on != motor_state:
            self._io_state.motor_on = motor_state
            self._io_state_pub.publish(self._io_state)

    def _setup_gpio(self) -> None:
        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self._pins.MOTOR_ON, GPIO.OUT)
        GPIO.setup(self._pins.STAGE2_INPUT, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)


def main():
    relays_node = RelaysNode('relays_node')
    rospy.spin()


if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
    finally:
        GPIO.cleanup()
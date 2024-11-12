import math as m

from geometry_msgs.msg import Twist

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from sensor_msgs.msg import LaserScan
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped


class FrameListener(Node):

    def __init__(self):
        super().__init__('ScanListen')

        self.publisher = self.create_publisher(Twist, '/robot/cmd_vel', 10)
        self.subscriber = self.create_subscription(LaserScan, '/robot/scan', self.update_pose, 10)
        self.scan = LaserScan()
        self.timer = self.create_timer(0.1, self.move)

    def update_pose(self, ranges):
        self.scan = ranges

    def move(self):
         
        vel_msg = Twist()
        laser = self.scan.ranges

        if len(laser)!=0:

            if laser[179] < 0.41:
                
                vel_msg.linear.x = 0.
            
            else:
                vel_msg.linear.x = 0.8
         
        self.publisher.publish(vel_msg)


def main():
    rclpy.init()
    node = FrameListener()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()
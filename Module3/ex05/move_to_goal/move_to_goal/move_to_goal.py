import rclpy
import sys
import time
from geometry_msgs.msg import Twist
from turtlesim.msg import Pose
from rclpy.node import Node
import numpy as np

class MoveToGoal(Node):
    def __init__(self, goal_x, goal_y, goal_theta):
        super().__init__('move_to_goal')
        self.goal_x, self.goal_y, self.goal_theta = goal_x, goal_y, goal_theta
        self.subscription = self.create_subscription(Pose, '/turtle1/pose', self.check_pose_callback, 10)
        self.publisher_ = self.create_publisher(Twist, '/turtle1/cmd_vel', 10)


    def check_pose_callback(self, msg):

        vel = Twist()
        x_error = round(self.goal_x - msg.x, 4)
        y_error = round(self.goal_y - msg.y, 4)
        goal_theta = np.arctan2(y_error, x_error)
        s = np.sqrt(x_error**2 + y_error**2)
        if s < 0.02:
            goal_theta = self.goal_theta*np.pi/180

            theta_error = goal_theta - msg.theta
            vel.angular.z = theta_error
            vel.linear.x = 0.0
            self.get_logger().info(f"i got:{self.goal_x} {self.goal_y} {msg.x} {msg.y} {msg.theta * 180 / np.pi}\n "
                                   f"i counted: {theta_error * 180 / np.pi} {x_error} {y_error}\n {goal_theta * 180 / np.pi}")
            self.publisher_.publish(vel)
            self.destroy_node()
            rclpy.shutdown()
        theta_error = goal_theta - msg.theta
        self.get_logger().info(f"i got:{self.goal_x} {self.goal_y} {msg.x} {msg.y} {msg.theta*180/np.pi}\n "
                               f"i counted: {theta_error*180/np.pi} {x_error} {y_error}\n {goal_theta*180/np.pi}")

        vel.angular.z = 6 * theta_error
        vel.linear.x = 1.5
        self.publisher_.publish(vel)





def main(args=None):
    rclpy.init(args=args)

    move_to_goal = MoveToGoal(float(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3]))

    rclpy.spin(move_to_goal)

    move_to_goal.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
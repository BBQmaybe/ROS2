import math as m

from geometry_msgs.msg import Twist, Pose, Pose2D

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped

from turtlesim.srv import Spawn

def quaternion_from_euler(ai, aj, ak):
    ai /= 2.0
    aj /= 2.0
    ak /= 2.0
    ci = math.cos(ai)
    si = math.sin(ai)
    cj = math.cos(aj)
    sj = math.sin(aj)
    ck = math.cos(ak)
    sk = math.sin(ak)
    cc = ci*ck
    cs = ci*sk
    sc = si*ck
    ss = si*sk

    q = np.empty((4, ))
    q[0] = cj*sc - sj*cs
    q[1] = cj*ss + sj*cc
    q[2] = cj*cs - sj*sc
    q[3] = cj*cc + sj*ss

    return q

def euler_from_quaternion(q):
    angles = [0] * 3

    sinr_cosp = 2 * (q.w * q.x + q.y * q.z)
    cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y)

    angles[0] = m.atan2(sinr_cosp, cosr_cosp)

    sinp = m.sqrt(1 + 2 * (q.w * q.y - q.x * q.z))
    cosp = m.sqrt(1 - 2 * (q.w * q.y - q.x * q.z))

    angles[1] = 2 * m.atan2(sinp, cosp) - m.pi / 2

    siny_cosp = 2 * (q.w * q.z + q.x * q.y)
    cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z)

    angles[2] = m.atan2(siny_cosp, cosy_cosp)

    return angles




class FrameListener(Node):

    def __init__(self):
        super().__init__('robot_frame_listener')

        self.cur_pose_ = Pose()

        self.listener = self.create_subscription(
            Odometry,
            "/robot/odom",
            self.on_timer,
            1)
        self.publisher = self.create_publisher(Twist, '/robot/cmd_vel', 1)
        # self.timer = self.create_timer(0.1, self.on_timer)
        self.movement = None
        self.is_moving = False
        self.goal_point = None

    def euclidean_distance(self, goal_pose):
        """Euclidean distance between current pose and the goal."""
        return ((goal_pose.position.x - self.cur_pose_.position.x)**2 +
                (goal_pose.position.y - self.cur_pose_.position.y)**2)**0.5

    def linear_vel(self, goal_pose, constant=1.5):
        return constant * self.euclidean_distance(goal_pose)
    
    def steering_angle(self, goal_pose):
       """See video: https://www.youtube.com/watch?v=Qh15Nol5htM."""
       return m.atan2(goal_pose.position.y - self.cur_pose_.position.y, goal_pose.position.x - self.cur_pose_.position.x)

    def angular_vel(self, goal_pose, z, constant=6):
       """See video: https://www.youtube.com/watch?v=Qh15Nol5htM."""
       return constant * (self.steering_angle(goal_pose) - z)

    def mov_to_point(self, msg):
        

        goal_pose = Pose()
        goal_pose.position.x = self.goal_point[0]
        goal_pose.position.y = self.goal_point[1]

        x, y, z = euler_from_quaternion(msg.pose.pose.orientation)

        self.cur_pose_ = msg.pose.pose

        vel_msg = Twist()
        tolerance = 0.1
        linear_vel_coef = 0.12
        angular_vel_coef = 1
        if self.euclidean_distance(goal_pose) >= tolerance:
            print(self.euclidean_distance(goal_pose))
            vel_msg.linear.x = linear_vel_coef * self.linear_vel(goal_pose)
            vel_msg.angular.z = angular_vel_coef * self.angular_vel(goal_pose, z=z)
            self.publisher.publish(vel_msg)
        else:
            vel_msg.linear.x = 0.
            vel_msg.angular.z = 0.
            self.publisher.publish(vel_msg)
            self.movement = None
            self.is_moving = False

    def on_timer(self, msg):
        if self.movement == None:
            self.movement = int(input("Choose movement: 1, 2, 3, 4\n"))
        match self.movement:
            case 1:
                if not self.is_moving:
                    self.get_logger().info(
                            f'Chose movement 1, please wait.')
                    self.goal_point = float(input("X_coord: ")), float(input("Y_coord: "))
                    self.is_moving = True
                    # self.get_logger().info(f"{self.last_msg.pose.pose.position}")
                self.mov_to_point(msg)
            case _:
                self.get_logger().info(
                        f'Movement not found.')
 

        # self.publisher.publish(msg)



def main():
    rclpy.init()
    node = FrameListener()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()

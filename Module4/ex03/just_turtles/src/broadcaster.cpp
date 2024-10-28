#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"
#include "turtlesim/msg/pose.hpp"

class FramePublisher : public rclcpp::Node
{
public:
    FramePublisher() : Node("turtle_tf2_frame_publisher")
    {
        turtle_name = this->declare_parameter<std::string>("turtlename", "turtle");

        tf_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        std::string topic_name = "/" + turtle_name + "/pose";

        subscription = this->create_subscription<turtlesim::msg::Pose>(topic_name, 10,
                std::bind(&FramePublisher::handle_turtle_pose, this, std::placeholders::_1));
    }

private:
    void handle_turtle_pose(const std::shared_ptr<turtlesim::msg::Pose> msg)
    {
        geometry_msgs::msg::TransformStamped t;

        t.header.stamp = this->get_clock()->now();
        t.header.frame_id = "world";
        t.child_frame_id = turtle_name.c_str();

        t.transform.translation.x = msg->x;
        t.transform.translation.y = msg->y;
        t.transform.translation.z = 0.0;

        tf2::Quaternion q;

        q.setRPY(0, 0, msg->theta);
        
        t.transform.rotation.x = q.x();
        t.transform.rotation.y = q.y();
        t.transform.rotation.z = q.z();
        t.transform.rotation.w = q.w();

        tf_broadcaster->sendTransform(t);
    }

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscription;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
    std::string turtle_name;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<FramePublisher>());
    
    rclcpp::shutdown();

    return 0;
}
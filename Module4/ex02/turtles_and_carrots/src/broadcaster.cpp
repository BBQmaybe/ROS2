#include <memory>
#include <string>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"
#include "turtlesim/msg/pose.hpp"

class FramePublisher : public rclcpp::Node
{
public:
	FramePublisher() : Node("frame_publisher")
	{
		turtle_name 	= this->declare_parameter<std::string>("turtlename", "turtle");
		tf_broadcaster 	= std::make_unique<tf2_ros::TransformBroadcaster>(*this);
		
		std::string topic_name = "/" + turtle_name + "/pose";		

		subscription = this->create_subscription<turtlesim::msg::Pose>(topic_name, 10, 
						std::bind(&FramePublisher::handle_turtle_pose, this, std::placeholders::_1));
	}

private:
	rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscription;
	std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
	std::string turtle_name;
	
	void handle_turtle_pose(const std::shared_ptr<turtlesim::msg::Pose> msg)
	{
		geometry_msgs::msg::TransformStamped transform_msg;
		
		transform_msg.header.stamp = this->get_clock()->now();
		transform_msg.header.frame_id = "world";
		transform_msg.child_frame_id = turtle_name.c_str();
		
		transform_msg.transform.translation.x = msg->x;
		transform_msg.transform.translation.y = msg->y;
		transform_msg.transform.translation.z = 0;
		
		tf2::Quaternion q;

		q.setRPY(0, 0, msg->theta);

		transform_msg.transform.rotation.x = q.x();
		transform_msg.transform.rotation.y = q.y();
		transform_msg.transform.rotation.z = q.z();
		transform_msg.transform.rotation.w = q.w();
		
		tf_broadcaster->sendTransform(transform_msg);
	}
};

int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);

	rclcpp::spin(std::make_shared<FramePublisher>());

	rclcpp::shutdown();

	return 0;
}


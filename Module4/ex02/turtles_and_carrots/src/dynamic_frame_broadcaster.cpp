#include <chrono>
#include <functional>
#include <memory>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.h"

using namespace std::chrono_literals;

const double PI = 3.141592653589793238463;

class DynamicFrameBroadcaster : public rclcpp::Node
{
public:
	DynamicFrameBroadcaster() : Node("dynamic_frame_broadcaser")
	{
		timer 			= this->create_wall_timer(100ms, std::bind(&DynamicFrameBroadcaster::broadcast_timer_callback, this));
		radius 			= this->declare_parameter<double>("radius");
		direction 		= this->declare_parameter<int>("direction_of_rotation");
		tf_broadcaster 	= std::make_shared<tf2_ros::TransformBroadcaster>(this);
	}

private:
	double radius;
	int direction;
	
	rclcpp::TimerBase::SharedPtr timer;
	std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
	
	void broadcast_timer_callback()
	{
		geometry_msgs::msg::TransformStamped transform_msg;
		
		rclcpp::Time now = this->get_clock()->now();

		transform_msg.header.stamp 		= now;
		transform_msg.header.frame_id 	= "turtle1";
		transform_msg.child_frame_id 	= "carrot1";
		
		double x = 0.2 * now.seconds() * PI;

		float x_move, y_move;
		if (direction == 1)
		{
			x_move = sin(x) * radius;
			y_move = cos(x) * radius;
		}
		else
		{
			x_move = cos(x) * radius;
			y_move = sin(x) * radius;
		}

		transform_msg.transform.translation.x = x_move;
    	transform_msg.transform.translation.y = y_move;
    	transform_msg.transform.translation.z = 0.0;
    	
    	transform_msg.transform.rotation.x 	= 0.0;
    	transform_msg.transform.rotation.y = 0.0;
    	transform_msg.transform.rotation.z = 0.0;
    	transform_msg.transform.rotation.w = 1.0;
    	
    	tf_broadcaster->sendTransform(transform_msg);
	}
};

int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);

	rclcpp::spin(std::make_shared<DynamicFrameBroadcaster>());

	rclcpp::shutdown();

	return 0;
}



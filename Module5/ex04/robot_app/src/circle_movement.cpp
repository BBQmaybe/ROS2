#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class GoRoundCirlce : public rclcpp::Node
{
public:
	GoRoundCirlce() : Node("round_circle")
	{
		publisher = this->create_publisher<geometry_msgs::msg::Twist>("/robot/cmd_vel", 10);
		timer = this->create_wall_timer(500ms, std::bind(&GoRoundCirlce::timer_callback, this));

		linear_speed = this->declare_parameter<double>("linear_speed");
		angular_speed = this->declare_parameter<double>("angular_speed");
	}

private:
	rclcpp::TimerBase::SharedPtr timer;
	rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher;

	double linear_speed, angular_speed;
	
	void timer_callback()
	{
		geometry_msgs::msg::Twist twist;
		
		twist.linear.x = this->linear_speed;
		twist.angular.z = this->angular_speed;
		
		publisher->publish(twist);
	}
};

int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);

	rclcpp::spin(std::make_shared<GoRoundCirlce>());

	rclcpp::shutdown();
    
	return 0;
}

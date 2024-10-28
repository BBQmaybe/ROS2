#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"

using namespace std::chrono_literals;

class FrameListener : public rclcpp::Node
{
public:
    FrameListener() : Node("turtle_tf2_frame_listener"), turtle_spawning_service_ready_(false), turtle_spawned_(false)
    {
        target_frame = this->declare_parameter<std::string>("target_frame", "turtle1");
        delay = this->declare_parameter<int>("delay", 0);

        RCLCPP_INFO(this->get_logger(), "%d\n", delay);
    
        tf_buffer      = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener    = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

        spawner    = this->create_client<turtlesim::srv::Spawn>("spawn");
        publisher  = this->create_publisher<geometry_msgs::msg::Twist>("turtle2/cmd_vel", 1);
        timer      = this->create_wall_timer(1s, std::bind(&FrameListener::on_timer, this));
    }

private:
    int delay;
    
    void on_timer()
    {
        std::string fromFrameRel = target_frame.c_str();
        std::string toFrameRel = "turtle2";

        if (turtle_spawning_service_ready_) 
        {
            if (turtle_spawned_) 
            {
                geometry_msgs::msg::TransformStamped transform_msg;

                try 
                {
                    rclcpp::Time now = this->get_clock()->now();
                    rclcpp::Time when = now - rclcpp::Duration(5, 0);
                    transform_msg = tf_buffer->lookupTransform(toFrameRel, now, fromFrameRel, when, "world", 50ms);
                } 
                catch (const tf2::TransformException & ex) 
                {
                    RCLCPP_INFO(
                        this->get_logger(), "Could not transform %s to %s: %s",
                        toFrameRel.c_str(), fromFrameRel.c_str(), ex.what());
                    return;
                }

                geometry_msgs::msg::Twist msg;

                static const double scaleRotationRate = 1.0;
                msg.angular.z = scaleRotationRate * atan2(
                    transform_msg.transform.translation.y,
                    transform_msg.transform.translation.x);

                static const double scaleForwardSpeed = 0.5;
                msg.linear.x = scaleForwardSpeed * sqrt(
                    pow(transform_msg.transform.translation.x, 2) +
                    pow(transform_msg.transform.translation.y, 2));

                publisher->publish(msg);
            } 
            else 
            {
                RCLCPP_INFO(this->get_logger(), "Successfully spawned");
                turtle_spawned_ = true;
            }
        } 
        else 
        {
            if (spawner->service_is_ready()) 
            {
                auto request = std::make_shared<turtlesim::srv::Spawn::Request>();

                request->name = "turtle2";
                request->x = 4.0;
                request->y = 2.0;
                request->theta = 0.0;

                using ServiceResponseFuture = rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture;

                auto response_received_callback = [this](ServiceResponseFuture future) 
                {
                    auto result = future.get();
                    if (strcmp(result->name.c_str(), "turtle2") == 0) 
                    {
                        turtle_spawning_service_ready_ = true;
                    } 
                    else 
                    {
                        RCLCPP_ERROR(this->get_logger(), "Service callback result mismatch");
                    }
                };
                auto result = spawner->async_send_request(request, response_received_callback);
            } 
            else 
            {
                RCLCPP_INFO(this->get_logger(), "Service is not ready");
            }
        }
    }
    bool turtle_spawning_service_ready_;
    bool turtle_spawned_;

    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawner{nullptr};
    rclcpp::TimerBase::SharedPtr timer{nullptr};
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher{nullptr};

    std::shared_ptr<tf2_ros::TransformListener> tf_listener{nullptr};
    std::unique_ptr<tf2_ros::Buffer> tf_buffer;

    std::string target_frame;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<FrameListener>());

    rclcpp::shutdown();

    return 0;
}
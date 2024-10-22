#include <functional>
#include <memory>
#include <thread>
#include <cstring>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim/msg/pose.hpp"

#include "turtle_action/action/message_turtle_action.hpp"

using namespace std::placeholders;

namespace action_turtle_commands
{
    class TurtleCmdActionServer : public rclcpp::Node
    {
    public:
    
        using action_ms = turtle_action::action::MessageTurtleAction;
        using GoalHandleMessageTurtleCommands = rclcpp_action::ServerGoalHandle<action_ms>;
        
        TurtleCmdActionServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions()) : Node("action_server", options)
            {
        
            this->_action_server = rclcpp_action::create_server<action_ms>(
                this, "turtle_commands_execute",
                std::bind(&TurtleCmdActionServer::handle_goal, this, _1, _2),
                std::bind(&TurtleCmdActionServer::handle_cancel, this, _1),
                std::bind(&TurtleCmdActionServer::handle_accepted, this, _1)
                );
            
            _publisher      = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
            _subscription   = this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10, std::bind(&TurtleCmdActionServer::updatePoseCallback, this, _1));
        }

    private:
       void updatePoseCallback(const turtlesim::msg::Pose& message)
        {
            _current_position = message;
        }

        typedef struct 
        {
            const std::shared_ptr<GoalHandleMessageTurtleCommands>  goal_handle;
            geometry_msgs::msg::Twist                               request;
            const std::shared_ptr<action_ms::Result>                result;
            const std::shared_ptr<action_ms::Feedback>              feedback;
        } Content;
    
        rclcpp_action::Server<action_ms>::SharedPtr             _action_server;
        rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr _publisher;
        rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr   _subscription;

        turtlesim::msg::Pose _init_pose;
        turtlesim::msg::Pose _current_position;

        rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const action_ms::Goal> goal)
        {
            RCLCPP_INFO(this->get_logger(), "Received goal request with command %s", goal->command.c_str());
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

        rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleMessageTurtleCommands> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        void handle_accepted(const std::shared_ptr<GoalHandleMessageTurtleCommands> goal_handle)
        {
            // this needs to return quickly to avoid blocking the executor, so spin up a new thread
            std::thread{std::bind(&TurtleCmdActionServer::execute, this, _1), goal_handle}.detach();
        }

        void turn(Content& ctn, float theta)
        {
            _init_pose = _current_position;
            
            ctn.request.linear.x = 0;
            ctn.request.angular.z = theta / 180 * 3.14;
            _publisher->publish(ctn.request);
            
            RCLCPP_INFO(this->get_logger(), "Turn %f", theta);
            ctn.result->result = true;
        }
    
        void move_forward(Content& ctn, float s, rclcpp::Rate& loop_rate)
        {
            float odom = 0;
            _init_pose = _current_position;
            auto request = geometry_msgs::msg::Twist();
            request.linear.x = s;

            _publisher->publish(request);
            
            while(abs(s - odom) > 0.02 && rclcpp::ok())
            {
                if (ctn.goal_handle->is_canceling())
                {
                    ctn.result->result = false;
                    _publisher->publish(geometry_msgs::msg::Twist());
                    ctn.goal_handle->canceled(ctn.result);
                    RCLCPP_INFO(this->get_logger(), "Goal cancelled, movement stopped");
                    return;
                }
            
                odom = std::sqrt(pow(_current_position.x - _init_pose.x, 2) + pow(_current_position.y - _init_pose.y, 2));
                ctn.feedback->odom = odom;

                ctn.goal_handle->publish_feedback(ctn.feedback);

                RCLCPP_INFO(this->get_logger(), "Publish Feedback %lf", odom);
                
                loop_rate.sleep();
            }

            RCLCPP_INFO(this->get_logger(), "Publish Feedback %lf", odom);
        }
    
        void execute(const std::shared_ptr<GoalHandleMessageTurtleCommands> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Executing goal");
            rclcpp::Rate loop_rate(10);

            const auto goal = goal_handle->get_goal();
            auto result     = std::make_shared<action_ms::Result>();

            Content ctn = 
            {
                goal_handle,
                geometry_msgs::msg::Twist(),
                std::make_shared<action_ms::Result>(),
                std::make_shared<action_ms::Feedback>(),
            };
            
            if (goal->command == "turn_right")
            {
                turn(ctn, -goal->angle);
            }
            else if (goal->command == "turn_left")
            {
                turn(ctn, goal->angle);
            }
            else if (goal->command == "forward")
            {
                move_forward(ctn, goal->s, loop_rate);
            }
            else if (strcmp(goal->command.c_str(), "") != 0)
            {
                if (goal->s != 0)
                {
                    move_forward(ctn, goal->s, loop_rate);
                }
                if (goal->angle != 0)
                {
                    turn(ctn, goal->angle);
                }
            }
            else
            {
                result->result = false;
                RCLCPP_INFO(this->get_logger(), "Unknown command or zero arguements %s", goal->command.c_str());
            }
        
            if (rclcpp::ok()) 
            {
                result->result = true;
                goal_handle->succeed(result);
                RCLCPP_INFO(this->get_logger(), "Goal succeeded");
            }
            else
            {
                result->result = true;
                goal_handle->succeed(result);
                RCLCPP_INFO(this->get_logger(), "Goal failed");
            } 
        }
    };  
}  

RCLCPP_COMPONENTS_REGISTER_NODE(action_turtle_commands::TurtleCmdActionServer);
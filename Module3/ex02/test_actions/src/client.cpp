#include <functional>
#include <future>
#include <memory>
#include <string>
#include <sstream>
#include <queue>
#include <chrono>
#include <thread>


#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "turtle_action/action/message_turtle_action.hpp"

using namespace std::placeholders;

namespace action_turtle_commands
{
    class TurtleCmdActionClient : public rclcpp::Node
    {
    public:
        using ActionMessage = turtle_action::action::MessageTurtleAction;
        using GoalHandleMessageTurtleCommands = rclcpp_action::ClientGoalHandle<ActionMessage>;

        TurtleCmdActionClient(std::vector<turtle_action::action::MessageTurtleAction::Goal> goals,
                                    const rclcpp::NodeOptions& options = rclcpp::NodeOptions()): Node("action_client", options)
        {
        
            this->client = rclcpp_action::create_client<ActionMessage>(this, "turtle_commands_execute");

            if (!this->client->wait_for_action_server())
            {
                RCLCPP_ERROR(this->get_logger(), "Action server is now not active");
                rclcpp::shutdown();
            }
            
            auto send_goal_options = rclcpp_action::Client<ActionMessage>::SendGoalOptions();

            send_goal_options.goal_response_callback    = std::bind(&TurtleCmdActionClient::goal_response_callback, this, _1);
            send_goal_options.feedback_callback         = std::bind(&TurtleCmdActionClient::feedback_callback, this, _1, _2);
            send_goal_options.result_callback           = std::bind(&TurtleCmdActionClient::result_callback, this, _1);
            
            for (auto command : goals)
            {
                RCLCPP_INFO(this->get_logger(), "Sending goal");

                //this->client_ptr_->async_send_goal(command, send_goal_options);

                auto f = this->client->async_send_goal(command, send_goal_options);
                //rclcpp::spin_until_future_complete(shared_from_this(), f);

                std::chrono::milliseconds timespan(2000); 
                std::this_thread::sleep_for(timespan);

                // while(true)
                // {
                //     if (f.wait_for(std::chrono::seconds(1)) == std::future_status::ready) 
                //     {
                //         break;
                //     }
                // }
            }

        }

    private:
        rclcpp_action::Client<ActionMessage>::SharedPtr client;
    
        void goal_response_callback(const GoalHandleMessageTurtleCommands::SharedPtr & goal_handle)
        {
            if (!goal_handle) 
            {
                RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
            } 
            else 
            {
                RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
            }
        }

        void feedback_callback(GoalHandleMessageTurtleCommands::SharedPtr, const std::shared_ptr<const ActionMessage::Feedback> feedback)
        {
            RCLCPP_INFO(this->get_logger(), "Got distance %d", feedback->odom);
        }

        void result_callback(const GoalHandleMessageTurtleCommands::WrappedResult& result)
        {
            switch (result.code) 
            {
            case rclcpp_action::ResultCode::SUCCEEDED:
            break;

            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
                return;

            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
                return;

            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Result: [%s]", result.result ? "true" : "false");
        }
    };  
}  


int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    
    auto goals = std::vector<turtle_action::action::MessageTurtleAction::Goal>();

    auto goal_1 = turtle_action::action::MessageTurtleAction::Goal();
    goal_1.command = "forward";
    goal_1.s = 2;
    goal_1.angle = 0;
    goals.push_back(goal_1);

    auto goal_2 = turtle_action::action::MessageTurtleAction::Goal();
    goal_2.command = "turn_right";
    goal_2.s = 0;
    goal_2.angle = 90;
    goals.push_back(goal_2);

    auto goal_3 = turtle_action::action::MessageTurtleAction::Goal();
    goal_3.command = "forward";
    goal_3.s = 1;
    goal_3.angle = 0;
    goals.push_back(goal_3);

    auto client = std::make_shared<action_turtle_commands::TurtleCmdActionClient>(goals);

    rclcpp::spin(client);
    rclcpp::shutdown();

    return 0;    
}
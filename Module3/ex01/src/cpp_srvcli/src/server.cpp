#include <rclcpp/rclcpp.hpp>
#include "full_name_service/srv/full_name_sum_service.hpp" 
 

class ServerNode : public rclcpp::Node {
public:
    ServerNode() : Node("service_name") {
        server_ = this->create_service<full_name_service::srv::FullNameSumService>("SummFullName",
            [this](const std::shared_ptr<full_name_service::srv::FullNameSumService::Request> request,
                   std::shared_ptr<full_name_service::srv::FullNameSumService::Response> response) {
                response->full_name = request->last_name + " " + request->first_name + " " + request->name;
            });
    }

private:
    rclcpp::Service<full_name_service::srv::FullNameSumService>::SharedPtr server_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ServerNode>());
    rclcpp::shutdown();
    return 0;
}

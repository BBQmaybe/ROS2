#include <rclcpp/rclcpp.hpp>
#include "full_name_service/srv/full_name_sum_service.hpp"


int main(int argc, char** argv) 
{
    rclcpp::init(argc, argv);

    if (argc != 4) 
    {
        RCLCPP_INFO(rclcpp::get_logger("client_node"), "Использование: client_node <last_name> <first_name> <middle_name>");
        return 1;
    }

    auto client_node = std::make_shared<rclcpp::Node>("client_name");
    auto client = client_node->create_client<full_name_service::srv::FullNameSumService>("SummFullName");

    while (!client->wait_for_service(std::chrono::seconds(1))) 
    {
        if (!rclcpp::ok()) 
        {
            RCLCPP_ERROR(rclcpp::get_logger("client_node"), "Прервано");
            return 1;
        }

        RCLCPP_INFO(rclcpp::get_logger("client_node"), "Ожидание сервиса 'SummFullName'...");
    }

    auto request = std::make_shared<full_name_service::srv::FullNameSumService::Request>();

    request->last_name  = argv[1];
    request->first_name = argv[2];
    request->name       = argv[3];

    auto result = client->async_send_request(request);

    rclcpp::spin_until_future_complete(client_node, result);
   

    if (rclcpp::spin_until_future_complete(client_node, result) == rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_INFO(rclcpp::get_logger("client_node"), "Результат: %s", result.get()->full_name.c_str());
    }
    else
    {
        RCLCPP_ERROR(rclcpp::get_logger("client_node"), "Не удалось выполнить запрос");
    }

    rclcpp::shutdown();

    return 0;
}

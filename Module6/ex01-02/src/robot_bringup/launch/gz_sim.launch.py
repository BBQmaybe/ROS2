from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # Получите путь к директории `ros_gz_sim`
    pkg_ros_gz_sim = get_package_share_directory('robot_bringup')
    
    # Описание запуска Gazebo с миром gpu_lidar_sensor.sdf
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_ros_gz_sim, "worlds", "gpu_lidar_sensor.sdf"),
        ),
        launch_arguments={
            "gz_args": "-r gpu_lidar_sensor.sdf"
        }.items(),
    )

    return LaunchDescription([gazebo])



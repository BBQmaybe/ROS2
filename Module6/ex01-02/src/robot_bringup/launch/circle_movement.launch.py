import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    robot_nodes = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('robot_bringup'), 'launch'),
                                       '/diff_drive.launch.py']),
    )


    return LaunchDescription([
        robot_nodes,
        DeclareLaunchArgument(
            'radius', default_value="5.0",
            description='Radius of carrot.'
        ),
        DeclareLaunchArgument(
            'direction_of_rotation', default_value='-1',
            description='Direction of rotation of the carrot.'
        ),
        Node(
            package='movement_circle',
            executable='move',
            name='mvm',
            parameters=[
                {
                    'radius': LaunchConfiguration('radius'),
                },
                {
                    "direction_of_rotation": LaunchConfiguration('direction_of_rotation')
                }
            ]
        ),
    ])

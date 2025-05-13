import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, ThisLaunchFileDir


def generate_launch_description():
    """
    Generate launch description for all components.

    Includes:
    - base.launch.py for sensors and perception
    - object_detection.launch.py (started immediately)
    - state_machine.launch.py (started after 10 seconds delay)
    """
    # Declare launch arguments
    debug_arg = DeclareLaunchArgument(
        "debug", default_value="false", description="Enable debug mode for all nodes"
    )

    # Create launch configuration variables
    debug = LaunchConfiguration("debug")

    # Get package directories
    smarty_utils_dir = get_package_share_directory("smarty_utils")
    state_machine_dir = get_package_share_directory("state_machine")
    object_detection_dir = get_package_share_directory("object_detection")

    # Include base launch file (perception and planning nodes)
    base_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(smarty_utils_dir, "launch", "base.launch.py")
        ),
        launch_arguments={"debug": debug}.items(),
    )

    # Include object_detection launch file (started immediately)
    object_detection_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(object_detection_dir, "launch", "object_detection.launch.py")
        ),
        launch_arguments={"debug": debug}.items(),
    )

    # Include state_machine launch file with a 10-second delay
    delayed_state_machine = TimerAction(
        period=10.0,  # 10 seconds delay
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(state_machine_dir, "launch", "state_machine.launch.py")
                ),
                launch_arguments={"debug": debug}.items(),
            )
        ],
    )

    # Create and return launch description
    return LaunchDescription(
        [debug_arg, base_launch, object_detection_launch, delayed_state_machine]
    )

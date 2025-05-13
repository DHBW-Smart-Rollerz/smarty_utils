import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    """
    Generate launch description for base launch file.

    Includes:
    - camera_preprocessing launch file
    - lane_detection_ai launch file
    - pose_estimation launch file (delayed by ~5 seconds)
    - path_planning launch file (delayed by ~5 seconds)
    """
    # Declare launch arguments
    debug_arg = DeclareLaunchArgument(
        "debug", default_value="false", description="Enable debug mode for all nodes"
    )

    # Create launch configuration variables
    debug = LaunchConfiguration("debug")

    # Get package directories
    camera_preprocessing_dir = get_package_share_directory("camera_preprocessing")
    lane_detection_ai_dir = get_package_share_directory("lane_detection_ai")
    pose_estimation_dir = get_package_share_directory("pose_estimation")
    pathplanning_dir = get_package_share_directory("pathplanning")

    # Include camera_preprocessing launch file
    camera_preprocessing_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                camera_preprocessing_dir, "launch", "camera_preprocessing.launch.py"
            )
        ),
        launch_arguments={"debug": debug}.items(),
    )

    # Include lane_detection_ai launch file
    lane_detection_ai_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(lane_detection_ai_dir, "launch", "lane_detection_ai.launch.py")
        ),
        launch_arguments={"debug": debug}.items(),
    )
    # Include pose_estimation launch file with delay
    pose_estimation_launch = TimerAction(
        period=15.0,  # 5 seconds delay
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(
                        pose_estimation_dir, "launch", "pose_estimation.launch.py"
                    )
                ),
                launch_arguments={"debug": debug}.items(),
            )
        ],
    )

    # Include path_planning launch file with delay
    path_planning_launch = TimerAction(
        period=15.0,  # 5 seconds delay
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(pathplanning_dir, "launch", "pathplanning.launch.py")
                ),
                launch_arguments={"debug": debug}.items(),
            )
        ],
    )

    # Create and return launch description
    return LaunchDescription(
        [
            debug_arg,
            camera_preprocessing_launch,
            # lane_detection_ai_launch, # Not working yet
            pose_estimation_launch,
            path_planning_launch,
        ]
    )

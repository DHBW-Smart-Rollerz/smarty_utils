# Copyright (c) 2024 Smart Rollerz e.V.
# All rights reserved.

# This is a custom message for all the detected lanes of the lane-detection-ai solution
# The message contains three lane_msgs (left, center, right), each defining a single lane.

# Header of the camera image the lanes were detected in. The stamp is the capture time
# of the camera frame and identifies the frame through the whole pipeline.
std_msgs/Header header

Lane left
Lane center
Lane right

geometry_msgs/Vector3 trajectory_left
geometry_msgs/Vector3 trajectory_right

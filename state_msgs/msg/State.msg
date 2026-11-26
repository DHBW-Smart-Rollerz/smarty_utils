# Copyright (c) 2025 Smart Rollerz e.V.
# All rights reserved.

# The state of the environment, containing all detected objects

# Header of the newest camera image whose detections are contained in this state
std_msgs/Header header

TrackedObject[] tracked_objects # Array of tracked objects in the environment

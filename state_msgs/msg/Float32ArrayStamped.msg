# Copyright (c) 2026 Smart Rollerz e.V.
# All rights reserved.

# A flat float array with a header, the stamped counterpart of std_msgs/Float32MultiArray.
# The header is the one of the camera image the data was computed from, so its stamp
# (capture time of the camera frame) identifies the frame through the whole pipeline.
# The layout of data is defined by the topic (see the README of the publishing package).

std_msgs/Header header
float32[] data

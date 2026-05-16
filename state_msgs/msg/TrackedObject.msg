# Copyright (c) 2025 Smart Rollerz e.V.
# All rights reserved.

# This message contains the information about one tracked object.


uint32 tracked_id      # Unique ID of the tracked object
uint8  class_id        # Class ID (0=car, 1=pedestrian, 2=stop, ...)
float32 position_x     # Ego position x (millimeters)
float32 position_y     # Ego position y (millimeters)
float32 velocity_x     # Velocity x (millimeters per second)
float32 velocity_y     # Velocity y (millimeters per second)
float32 confidence     # Tracking confidence [0.0, 1.0]
float32 width          # Bounding box width (millimeters)

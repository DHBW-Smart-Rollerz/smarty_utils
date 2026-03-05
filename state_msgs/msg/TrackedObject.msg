# Copyright (c) 2025 Smart Rollerz e.V.
# All rights reserved.

# This message contains the information about one tracked object.


float64 tracked_id # Unique ID of the tracked object
float64 class_id # Class ID of the tracked object (e.g., 0 for car, 1 for pedestrian, etc.)
float64 position_x # Ego position in the x-axis (in millimeters)
float64 position_y # Ego position in the y-axis (in millimeters)
float64 velocity_x # Velocity in the x-axis (in millimeters per second)
float64 velocity_y # Velocity in the y-axis (in millimeters per second)
float64 confidence # Confidence score of the detection (between 0 and 1)
float64 width # Width of the bounding box (in millimeters)

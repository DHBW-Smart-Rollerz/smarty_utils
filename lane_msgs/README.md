# lane_msgs

This package contains the custom message definitions for the lane detection package.

## Custom Messages

### Lane.msg

The `Lane.msg` message contains information about a single driving lane. It includes:
- `points`: An array of `geometry_msgs/Vector3` representing the points that define the lane.
- `detected`: A boolean indicating whether the lane has been detected.

### LaneDetectionResult.msg

The `LaneDetectionResult.msg` message is used to represent the result of the lane detection AI solution. It includes:
- `header`: The header of the camera image the lanes were detected in (see "Frame timestamps" in the `state_msgs` README).
- `left`: A `Lane` message defining the left lane.
- `center`: A `Lane` message defining the center lane.
- `right`: A `Lane` message defining the right lane.
- `trajectory_left`: A `geometry_msgs/Vector3` representing the trajectory of the left lane.
- `trajectory_right`: A `geometry_msgs/Vector3` representing the trajectory of the right lane.

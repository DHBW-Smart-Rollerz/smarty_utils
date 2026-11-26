# state_msgs

This package contains the custom message definitions for communication between object tracking and the state machine, providing a comprehensive representation of the environment state.

## Custom Messages

### State.msg

The `State.msg` message represents the complete state of the environment at a given moment. It includes:
- `header`: The header of the newest camera image whose detections are contained in the state (see [Frame timestamps](#frame-timestamps)).
- `tracked_objects`: An array of `TrackedObject` messages containing all currently detected and tracked objects in the environment.

This message serves as a comprehensive snapshot of the surrounding environment, enabling the state machine to make informed decisions based on all detected objects.

### Float32ArrayStamped.msg

The stamped counterpart of `std_msgs/Float32MultiArray`, used for flat arrays computed from a camera image (object and sign detections, lane trajectories):
- `header`: The header of the camera image the data was computed from.
- `data`: The values (Float32[]); the layout is defined by the topic.

Subscribers that only read `msg.data` of a `Float32MultiArray` work unchanged after switching the message type.

### TrackedObject.msg

The `TrackedObject.msg` message contains detailed information about a single tracked object in the environment. It includes:
- `tracked_id`: A unique identifier for tracking this object across multiple frames (Float64).
- `class_id`: The classification identifier of the object (Float64), where values represent different traffic signs and objects according to the SIGNS enum.
- `position_x`: The object's position along the x-axis in the ego coordinate system, measured in millimeters (Float64).
- `position_y`: The object's position along the y-axis in the ego coordinate system, measured in millimeters (Float64).
- `velocity_x`: The object's velocity component along the x-axis, measured in millimeters per second (Float64).
- `velocity_y`: The object's velocity component along the y-axis, measured in millimeters per second (Float64).
- `confidence`: The detection confidence score, ranging from 0.0 to 1.0, where higher values indicate greater confidence (Float64).
- `width`: The width of the object's bounding box, measured in millimeters (Float64).

### Coordinate System

All spatial measurements use an ego-centric coordinate frame:
- **Origin**: Vehicle center
- **X-axis**: Forward direction (positive values = ahead of vehicle)
- **Y-axis**: Lateral direction (positive values = left of vehicle)
- **Units**: Millimeters for position and width, millimeters per second for velocity

### Common Class IDs (Traffic Signs)

| Class ID | Sign Type |
|----------|-----------|
| 1 | Stop Sign |
| 3 | No Overtaking |
| 4 | No Overtaking Lifted |
| 5 | Fast Track (Express Way) |
| 6 | Fast Track Lifted |
| 7 | Speed Limit 30 |
| 8 | Speed Limit 30 Lifted |
| 9 | Crosswalk |
| 13 | Priority Oncoming Traffic |
| 14 | Parking |
| 15 | Turn Left |
| 16 | Turn Right |
| 17 | Give Way |
| 18 | Priority Road |
| 19 | Pedestrian Island |

**Note**: The `class_id` field corresponds to the `SIGNS` enum values defined in the `smarty_utils` package.

## Frame timestamps

Every message computed from a camera image carries the `header` of that image. The
camera driver is the only node that creates a stamp (`header.stamp`, capture time of the
frame); every other node copies the header of its input message into its output message
(`out.header = msg.header`) and never sets `self.get_clock().now()` as stamp. The stamp
therefore identifies the camera frame a message was derived from, e.g. to find the image
of a steering decision in a rosbag or to measure the latency of the pipeline
(`now - msg.header.stamp`).

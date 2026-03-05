# state_msgs

This package contains the custom message definitions for communication between object tracking and the state machine, providing a comprehensive representation of the environment state.

## Custom Messages

### EnvironmentState.msg

The `EnvironmentState.msg` message represents the complete state of the environment at a given moment. It includes:
- `tracked_objects`: An array of `TrackedObject` messages containing all currently detected and tracked objects in the environment.

This message serves as a comprehensive snapshot of the surrounding environment, enabling the state machine to make informed decisions based on all detected objects.

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

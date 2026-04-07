from enum import Enum


class Nodes(Enum):
    """Computation Nodes."""

    LANE_DETECTION = "lane_detection_ai_node"
    OBJECT_DETECTION = "object_detection_node"
    PATH_PLANNING = "path_planning_node"
    CONTROL = "control_node"
    STATE_ESTIMATION = "state_estimation_node"
    TRACKING = "tracking_node"


class NodeState(Enum):
    """Describes the state of a node in the state machine."""

    INACTIVE = 0
    ACTIVE = 1
    RESET = 2


class Light(Enum):
    """Enum for light states."""

    BLINK_LEFT = 0b0100
    BLINK_RIGHT = 0b1000

    BRAKE = 0b0010
    BLINK_LEFT_BRAKE = 0b0110
    BLINK_RIGHT_BRAKE = 0b1010
    BLINK_LEFT_BRAKE_NORMAL = 0b0111
    BLINK_RIGHT_BRAKE_NORMAL = 0b1011

    NORMAL = 0b0001
    BLINK_RIGHT_NORMAL = 0b1001
    BLINK_LEFT_NORMAL = 0b0101
    BRAKE_NORMAL = 0b0011

    DRIVE_BACK = 0b10000
    DRIVE_BACK_BRAKE = DRIVE_BACK | BRAKE
    DRIVE_BACK_BLINK_LEFT = DRIVE_BACK | BLINK_LEFT
    DRIVE_BACK_BLINK_RIGHT = DRIVE_BACK | BLINK_RIGHT
    DRIVE_BACK_BLINK_LEFT_BRAKE = DRIVE_BACK | BLINK_LEFT | BRAKE
    DRIVE_BACK_BLINK_RIGHT_BRAKE = DRIVE_BACK | BLINK_RIGHT | BRAKE
    DRIVE_BACK_NORMAL = DRIVE_BACK | NORMAL
    DRIVE_BACK_BRAKE_NORMAL = DRIVE_BACK | BRAKE | NORMAL
    DRIVE_BACK_BLINK_LEFT_NORMAL = DRIVE_BACK | BLINK_LEFT | NORMAL
    DRIVE_BACK_BLINK_RIGHT_NORMAL = DRIVE_BACK | BLINK_RIGHT | NORMAL
    DRIVE_BACK_BLINK_LEFT_BRAKE_NORMAL = DRIVE_BACK | BLINK_LEFT | BRAKE | NORMAL
    DRIVE_BACK_BLINK_RIGHT_BRAKE_NORMAL = DRIVE_BACK | BLINK_RIGHT | BRAKE | NORMAL

    OFF = 0


class OBJECTS(Enum):
    """Enum for object types."""

    VEHICLE = 2
    PEDESTRIAN = 10
    INTERSECTION_LANE_TYPE_EGO_SOLID = 20
    INTERSECTION_LANE_TYPE_EGO_DOTTED = 21
    INTERSECTION_LANE_TYPE_OPP_SOLID = 22
    INTERSECTION_LANE_TYPE_OPP_DOTTED = 23
    INTERSECTION_LANE_TYPE_RIGHT_SOLID = 24
    INTERSECTION_LANE_TYPE_RIGHT_DOTTED = 25
    INTERSECTION_LANE_TYPE_LEFT_SOLID = 26
    INTERSECTION_LANE_TYPE_LEFT_DOTTED = 27


class SIGNS(Enum):
    """Enum for sign types."""

    STOP = 1
    NO_OVERTAKING = 3
    NO_OVERTAKING_LIFTED = 4
    FAST_TRACK = 5
    FAST_TRACK_LIFTED = 6
    SPEED_LIMIT_30 = 7
    SPEED_LIMIT_30_LIFTED = 8
    CROSSWALK = 9
    PRIORITY_ONCOMING_TRAFFIC = 13
    PARKING = 14
    TURN_LEFT = 15
    TURN_RIGHT = 16
    GIVE_WAY = 17
    PRIORITY = 18
    PEDESTRIAN_ISLAND = 19


class Location(Enum):
    """Location of the object."""

    LEFT_LANE = "left_lane"
    RIGHT_LANE = "right_lane"
    LEFT = "left"
    RIGHT = "right"
    OUTSIDE = "outside"
    FRONT = "front"
    BACK = "back"
    UNKNOWN = "unknown"
    NOT_RELEVANT = "not_relevant"

    @staticmethod
    def opposite(location: "Location", out_of_lane: bool = False) -> "Location":
        """
        Get the opposite location of a given location.

        Arguments:
            location -- Location to get the opposite of

        Returns:
            Location -- Opposite location
        """
        if location == Location.LEFT:
            return Location.RIGHT
        elif location == Location.RIGHT:
            return Location.LEFT
        elif location == Location.FRONT:
            return Location.BACK
        elif location == Location.BACK:
            return Location.FRONT
        elif location == Location.LEFT_LANE:
            if out_of_lane:
                return Location.RIGHT
            return Location.RIGHT_LANE
        elif location == Location.RIGHT_LANE:
            if out_of_lane:
                return Location.LEFT
            return Location.LEFT_LANE
        else:
            return location


class StateMachineTestModes(Enum):
    """Test modes for the state machine."""

    NORMAL = 0b00000000
    NO_STARTBOX = 0b00000010

    @staticmethod
    def use(value: int, test_mode: "StateMachineTestModes") -> bool:
        """Check if the test mode is active."""
        return (value & test_mode.value) != 0

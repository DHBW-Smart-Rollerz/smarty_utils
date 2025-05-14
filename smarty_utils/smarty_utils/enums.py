from enum import Enum


class Nodes(Enum):
    """Computation Nodes."""

    LANE_DETECTION = "lane_detection_ai_node"
    OBJECT_DETECTION = "object_detection_node"
    PATH_PLANNING = "path_planning_node"
    CONTROL = "control_node"
    STATE_ESTIMATION = "state_estimation_node"


class NodeState(Enum):
    """Describes the state of a node in the state machine."""

    INACTIVE = 0
    ACTIVE = 1
    RESET = 2


class Light(Enum):
    """Enum for light states."""

    BLINK_LEFT = 4
    BLINK_RIGHT = 3
    NORMAL = 1
    BRAKE = 2
    OFF = 0


class OBJECTS(Enum):
    """Enum for object types."""

    VEHICLE = 2
    PEDESTRIAN = 10


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

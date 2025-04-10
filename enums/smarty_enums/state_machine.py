from enum import Enum


class NodeState(Enum):
    """Describes the state of a node in the state machine."""

    INACTIVE = 0
    ACTIVE = 1
    RESET = 2


class GoalLane(Enum):
    """Describes the goal lane of the state machine."""

    RIGHT = 0
    LEFT = 1

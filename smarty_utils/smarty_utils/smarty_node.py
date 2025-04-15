import rclpy
from ament_index_python import get_package_share_directory
from rcl_interfaces.msg import SetParametersResult
from rclpy.node import Node
from rclpy.parameter import Parameter

from smarty_utils.enums import NodeState

QOS_PROFILE = rclpy.qos.QoSProfile(depth=1)


class SmartyNode(Node):
    """Base class for all nodes in the smarty project."""

    def __init__(
        self,
        name: str,
        package_name: str,
        node_parameters: dict[str, any] = {},
        subscribed_topics: dict[str, tuple[any, callable, int | None]] = {},
        published_topics: dict[str, tuple[any, int | None]] = {},
    ):
        """Initialize the SmartyNode."""
        super().__init__(name)
        self.node_parameters = node_parameters
        self.subscribed_topics = subscribed_topics
        self.published_topics = published_topics
        self.package_path = get_package_share_directory(package_name)

        # Init node
        self._init_parameters()
        self._init_publishers()
        self._init_subscribers()

        self.get_logger().info(f"Node {name} initialized.")

    @property
    def _debug(self) -> bool:
        """Return the debug parameter."""
        return self.get_parameter("debug").value

    @property
    def _state(self) -> NodeState:
        """Return the state parameter."""
        return NodeState(self.get_parameter("state").value)

    def reset(self) -> None:
        """Reset the node to its initial state."""
        self.get_logger().warning("⚠️ Resetting the node ...")
        self._reset()
        self.set_parameters([Parameter(name="state", value=NodeState.ACTIVE.value)])
        self.get_logger().info("✅ Node reset performed. Continue with ACTIVE state ...")

    def _reset(self) -> None:
        """Reset the node to its initial state."""
        self.get_logger().info("Override this method to reset the node.")

    def _init_parameters(self):
        """Initialize the parameters of the node."""
        for param_name, param_value in self.node_parameters.items():
            self.declare_parameter(param_name, param_value)

        if "debug" not in self.node_parameters.keys():
            self.declare_parameter("debug", False)
            self.node_parameters["debug"] = False
        if "state" not in self.node_parameters.keys():
            self.declare_parameter("state", NodeState.INACTIVE.value)
            self.node_parameters["state"] = NodeState.INACTIVE

        self.add_on_set_parameters_callback(self.parameter_change_callback)
        self.add_post_set_parameters_callback(self.post_parameter_change_callback)

    def _init_publishers(self):
        """Initialize the publishers of the node."""
        for key, (topic_type, qos) in self.published_topics.items():
            if key not in self.node_parameters.keys():
                raise ValueError(f"Parameter '{key}' not found in node parameters.")
            topic_name = self.get_parameter(key).value
            qos = QOS_PROFILE if qos is None else qos
            self.__setattr__(key, self.create_publisher(topic_type, topic_name, qos))

    def _init_subscribers(self):
        """Initialize the subscribers of the node."""
        for key, (topic_type, callback, qos) in self.subscribed_topics.items():
            if key not in self.node_parameters.keys():
                raise ValueError(f"Parameter '{key}' not found in node parameters.")
            topic_name = self.get_parameter(key).value
            qos = QOS_PROFILE if qos is None else qos
            self.__setattr__(
                key,
                self.create_subscription(topic_type, topic_name, callback, qos),
            )

    def parameter_change_callback(self, params: list[Parameter]) -> SetParametersResult:
        """
        Callback for the parameter server.

        Arguments:
            params -- changed params
        """
        for param in params:
            if self._debug:
                self.get_logger().info(
                    f"Parameter '{param.name}' changed to {param.value}"
                )
        return SetParametersResult(successful=True)

    def post_parameter_change_callback(self, params: list[Parameter]) -> None:
        """
        Post callback for the parameter server.

        Arguments:
            params -- Parameter list
        """
        for param in params:
            if param.name == "state" and param.value == NodeState.RESET.value:
                self.reset()

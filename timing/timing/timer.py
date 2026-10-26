import collections
import time
from collections import deque
from collections.abc import Callable
from contextlib import ContextDecorator
from dataclasses import dataclass, field
from typing import ClassVar, Self


class TimerError(Exception):
    """A custom exception used to report errors in use of Timer class."""


@dataclass
class Timer(ContextDecorator):
    """Times the code using a class, context manager, or decorator.

    Args:
        name: optional name of the timer, required if used as context manager or
            decorator
        log_on_stop: optional bool if the time should be logged directly after the
            measurement has finished
        filter_strength: optional int of over how many measurements should be
            smoothed
        text: optional text that should be displayed if log_on_stop is True
        logger: optional callable on which should be logged, default: print
    """

    timers: ClassVar[dict[str, collections.deque]] = {}
    name: str | None = None
    log_on_stop: bool | None = False
    filter_strength: int | None = 1
    text: str | None = "Elapsed time for '{0}': {1:0.1f} milliseconds"
    logger: Callable[[str], None] | None = print
    _start_time: float | None = field(default=None, init=False, repr=False)

    def __post_init__(self) -> None:
        """Initialization: add timer to dict of timers."""
        if self.name:
            self.timers.setdefault(self.name, deque(maxlen=self.filter_strength))

    def start(self) -> None:
        """Start a new timer."""
        if self._start_time is not None:
            raise TimerError("Timer is running. Use .stop() to stop it")

        self._start_time = time.perf_counter()

    def stop(self) -> float:
        """Stop the timer, and report the elapsed time."""
        if self._start_time is None:
            raise TimerError("Timer is not running. Use .start() to start it")

        # Calculate elapsed time in milliseconds
        elapsed_time = (time.perf_counter() - self._start_time) * 1000
        self._start_time = None

        # Append elapsed time to the dequeue of this timer
        if self.name:
            self.timers[self.name].append(elapsed_time)

        # Calculate average time
        avg_time = sum(t for t in self.timers.get(self.name)) / len(
            self.timers.get(self.name)
        )

        # Report elapsed time if required
        if self.log_on_stop and self.logger:
            self.logger(self.text.format(self.name, avg_time))

        return avg_time

    def print(self, timers: list | None = None) -> str:
        """Prints the time for all given timers.

        If None is passed, all timers will be reported.

        Args:
            timers: name of the timers that should be reported
        Returns:
            str: log string
        """
        # If no timers are passed, report all timers
        if timers is None:
            timers = self.timers.keys()

        log = ""
        for timer_name in timers:
            try:
                avg_time = sum(t for t in self.timers.get(timer_name)) / len(
                    self.timers.get(timer_name)
                )
            except ZeroDivisionError:
                avg_time = -1
            log += f"'{timer_name}': {avg_time:0.1f} "

        # Report the prepared string
        if self.logger:
            self.logger(log)

        return log

    def reset(self, timers: list | None = None) -> None:
        """Resets the time for all given timers.

        If None is passed, all timers will be reset.

        Args:
            timers: name of the timers that should be reset
        """
        # If no timers are passed, reset all timers
        if timers is None:
            timers = self.timers.keys()

        # Reset timers
        for timer_name in timers:
            self.timers[timer_name].clear()

    def __enter__(self) -> Self:
        """Start a new timer as a context manager."""
        self.start()
        return self

    def __exit__(self, *exc_info: object) -> None:
        """Stop the context manager timer."""
        self.stop()

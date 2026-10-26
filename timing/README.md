## Timer Utility

The `Timer`utility allows you to time your code using a class, context manager, or decorator. This is useful for measuring the performance of specific code blocks or functions.

### Usage

You can use the `Timer` in three different ways: as a class, a context manager, or a decorator.

#### As a Class

```python
timer = Timer(
    name="example_timer",
    log_on_stop=True,
    filter_strength=5,
    text="Elapsed time:",
    logger=print,
)
timer.start()
# Your code here
# ... repeat as often as needed
timer.stop()
```

#### As a Context Manager

```python
with Timer(name="example_timer", log_on_stop=True, filter_strength=5, text="Elapsed time:", logger=print):
    # Your code here
    #... repeat as often as needed
```

#### As a Decorator

```python
@Timer(name="example_timer", log_on_stop=True, filter_strength=5, text="Elapsed time:", logger=print)
def your_function():
    # Your code here
    #... repeat as often as needed

your_function()
```

### Arguments

- `name`: Optional name of the timer, required if used as a context manager or decorator.
- `log_on_stop`: Optional boolean indicating if the time should be logged directly after the measurement has finished.
- `filter_strength`: Optional integer specifying over how many measurements should be smoothed.
- `text`: Optional text that should be displayed if `log_on_stop` is `True`.
- `logger`: Optional callable to which the log should be sent, default is `print`.

### Example

```python
from timer import Timer

# Using as a class
timer = Timer(name="example_timer", log_on_stop=True, filter_strength=5, text="Elapsed time:", logger=print)
timer.start()
# Your code here
#... repeat as often as needed
timer.stop()

# Using as a context manager
with Timer(name="example_timer", log_on_stop=True, filter_strength=5, text="Elapsed time:", logger=print):
    # Your code here
    #... repeat as often as needed

# Using as a decorator
@Timer(name="example_timer", log_on_stop=True, filter_strength=5, text="Elapsed time:", logger=print)
def your_function():
    # Your code here
    #... repeat as often as needed

your_function()

# At the end, print the results to the console
Timer().print()
```

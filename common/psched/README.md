# PSched

This modules serves as our "RTOS", if you will. It replaces FreeRTOS with a simpler scheduling method that doesn't carry the bagage of priority issues or untraceable hard faults.
It should be relatively easy to implement. It's been tested a tad on an L4. F4 support hasn't been confirmed yet.

## Roadmap

- [X] Basic scheduling
- [X] Basic task creation
- [X] Basic background loop
- [X] Loop pausing
- [X] Timing miss detection
- [X] Task deletion
- [X] Watchdog implementation
- [ ] F4 support (written but untested)
- [ ] Free running counter rather than interrupt based
- [ ] Proper task execution time tracking

## Using PSched

Use is pretty simple. 3 function calls are required to start the scheduler. First, `schedInit()` should be called with the operating frequency of the MCU as an input argument.
This value can be found in the clock configuration portion of Cube. Next, `taskCreate()` needs to be called with the address of the function to schedule and a rate for the task
in milliseconds. Finally, `schedStart()` should be called to begin scheduler execution. This function will not return unless schedPause() is called for some reason. Here's an example.

```C++
schedInit(32000000);
taskCreate(blink, 500);
taskCreate(stop, 4000);
schedStart();
```

This setup assumes a base clock frequency of 32 MHz. A function named `blink()` will run every 500 ms, while a function named `stop()` will run every 4 seconds. Once `schedStart()` is
called, the scheduler will run until it is told otherwise.

## Function Descriptions

Doxygen comments are inserted, and the source files should be enough to understand usage, but they're documented here just to be sure.

### schedInit(uint32_t freq)

Initializes the scheduler. Input argument is base clock frequency (to timer 7). There is no need to call this after `schedPause()`.

>  NOT CALL `taskCreate()` prior to `schedInit()`! Your task will be removed upon calling `schedInit()`!

### taskCreate(func_ptr_t func, uint16_t task_time)

Initializes a task to run. `func` is the address of the function in question. `task_time` is the execution period in milliseconds.

### taskDelete(uint8_t type, uint8_t task)

Deletes a task and stops its execution. `type` is the type of task to delete. 0 marks it as a regular task; 1 marks it as a background task. `task` is the index of the task to remove.

### schedStart()

Starts the scheduler. Will not return unless `schedPause()` is called.

### schedPause()

Pauses the scheduler. Will allow `schedStart()` to return to `main()`, so if you need to use this, you'd better have a contingency once the function returns. After calling `schedPause()`,
the scheduler does not need to be re-initialized, nor to tasks need to be re-registered with the scheduler.

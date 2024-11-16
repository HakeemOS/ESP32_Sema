# _ESP 32 Series_

## Semaphore Project
Using semaphore to signal to 1 task when the other tasks have completed 

**Task One: Watcher 
- Received input from serial monitor and stores on heap to be used by _"Generators"_
- Use semaphores to track and report once all _"Generators"_ have completed Watcher-dependant function
- Self-Cleanup and Destruction

**Task Two: Generator 
- Wait for _"Watcher"_ to receive and store message
- Output message and release semaphore
- Enter a waiting state

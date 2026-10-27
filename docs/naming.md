<!-- covers: src/Nodens/Concurrency/, src/Nodens/Core/Application.cppm, src/Nodens/Rendering/, src/Platform/ -->

# Type names

Nodens names a type for its role, and the kind marker says what kind of declaration it is: `I` for an interface, `F` for a plain data struct, `E` for an enumeration. A rename changes the word inside the marker, never the marker itself.

The implementation carries the platform word and the interface keeps the role word: `IWindow` with `GlfwWindow`, and `IGraphicsBackend` with `OpenGLBackend` or `VulkanBackend`.

## Why the thread pool is `TaskSystem`

Task-based submission is one callable plus a handle to its result: `std::async` and `std::packaged_task` "shift the thread management responsibility to the implementer of the C++ Standard Library" and return a `std::future`.[^meyers] This pool does exactly that, one callable at a time.

A job system in the engine literature is a queue that worker threads drain, and in its simple form "every job must run to completion once it starts running".[^gregory-553] Letting a job sleep while it waits for another job needs coroutines or fibers, and joining a batch of jobs needs counters.[^gregory-counters] Naughty Dog's own job system is fiber-based.[^gregory-fibers]

Nodens hands back a `std::future`, and it has no counters, no sleeping jobs and no fibers, so the narrower word is the accurate one: it schedules tasks. `Application::GetTaskSystem()` follows the type.

## References

[^meyers]: Scott Meyers, *Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14* (Beijing; Sebastopol, CA: O'Reilly Media, Incorporated, 2014), Item 35.

[^gregory-553]: Jason Gregory, *Game Engine Architecture*, Third edition (Boca Raton London New York: CRC Press, Taylor & Francis Group, 2019), p. 553.

[^gregory-counters]: Gregory, *Game Engine Architecture*, pp. 554-555.

[^gregory-fibers]: Gregory, *Game Engine Architecture*, p. 554.

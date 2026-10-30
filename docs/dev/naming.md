<!-- covers: src/Nodens/Concurrency/, src/Nodens/Core/Application.cppm, src/Nodens/Rendering/, src/Platform/ -->

# Type names

A type is named for its role, and a leading letter says what kind of declaration it is: `I` for an interface, `F` for a plain data struct, `E` for an enumeration. A rename changes the word inside the marker and never the marker itself.

## The decision

The marker is the first character, and the role word follows it: `IWindow`, `IGraphicsBackend`, `FApplicationConfig`, `FWindowConfig`, `EGraphicsAPI`, `ETheme`.

The implementation carries the platform word and the interface keeps the role word, so `IWindow` is implemented by `GlfwWindow`, and `IGraphicsBackend` by `OpenGLBackend` or `VulkanBackend`.

## Where each name came from

### The graphics interface is a backend

`IGraphicsBackend` covers initialization and frame presentation, and nothing else. OpenGL presents by swapping its window buffers, and Vulkan presents through its swapchain; a context is an OpenGL object, so the shared word is backend, and `IWindow::GetBackend()` returns it.

### The theme enum is `ETheme`

The enum's first member is `None`, and the appliers are free functions named `ApplyDefaultDarkTheme` and `ApplyDefaultLightTheme`. "Default" describes what the appliers do, not what the enum holds, so the field on `FApplicationConfig` is `Theme` and the module keeps the word for its appliers.

### The startup structs are `*Config`

Both structs hold the values a caller sets before the application starts, and the window struct's own documentation called them configuration properties. Wherever one is held, the name is `config`: the constructor parameter, the window local `windowConfig`, and the examples' `appConfig`.

### The thread pool is `TaskSystem`

Task-based submission is one callable plus a handle to its result: `std::async` and `std::packaged_task` "shift the thread management responsibility to the implementer of the C++ Standard Library" and return a `std::future`.[^meyers] This pool does exactly that, one callable at a time.

A job system in the engine literature is a queue that worker threads drain, and in its simple form "every job must run to completion once it starts running".[^gregory-553] Letting a job sleep while it waits for another job needs coroutines or fibers, and joining a batch of jobs needs counters.[^gregory-counters] Naughty Dog's own job system is fiber-based.[^gregory-fibers]

Nodens hands back a `std::future`, and it has no counters, no sleeping jobs and no fibers, so the narrower word is the accurate one: it schedules tasks. `Application::GetTaskSystem()` follows the type.

## What it costs

- **A rename reaches every surface.** The interface file, its implementation, the examples, the template repository, the guide pages and the changelog all carry the old word, so a rename is one change across several repositories, not an edit.
- **The marker cannot be dropped later** without the same breadth of change, which is why a wrong marker is expensive rather than cosmetic.
- **A name whose role is still moving waits.** `ILayer` keeps its name while the layer system is redesigned, because a nearer role word would describe behaviour the code does not have yet, and the rename would have to be repeated.

## Changing a name

1. Decide the role the type plays, not the object it happens to hold, and keep the marker for its kind.
2. Rename the type, its accessor and every local that holds one, since the accessor names the type.
3. Update the examples, the template repository and the guide pages that name it.
4. Record the old and the new name in the changelog, under Changed.

## References

[^meyers]: Scott Meyers, *Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14* (Beijing; Sebastopol, CA: O'Reilly Media, Incorporated, 2014), Item 35.

[^gregory-553]: Jason Gregory, *Game Engine Architecture*, Third edition (Boca Raton London New York: CRC Press, Taylor & Francis Group, 2019), p. 553.

[^gregory-counters]: Gregory, *Game Engine Architecture*, pp. 554-555.

[^gregory-fibers]: Gregory, *Game Engine Architecture*, p. 554.

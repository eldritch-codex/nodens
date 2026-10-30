# Documentation

Three kinds of page, by who reads them: the guide for people building an application on Nodens, the design notes for people changing the framework, and the process pages for anyone contributing a change.

## Guide

Start at the [guide index](guide/README.md). The pages, in reading order:

- [Getting Started](guide/getting-started.md): a window on screen from the application template.
- [Architecture Overview](guide/architecture-overview.md): what the application owns and what one frame does.
- [Application Loop](guide/application-loop.md): one iteration, the pacing modes, and running with no window.
- [Layer System](guide/layer-system.md): the five layer callbacks and the stack they sit in.
- [Event System](guide/event-system.md): routed input and the pub/sub event bus.
- [Concurrency and TaskSystem](guide/concurrency-and-tasksystem.md): background work and how results come back.
- [Creating a Custom Layer](guide/creating-a-custom-layer.md): one layer built from scratch.
- [Using ImGui and ImPlot](guide/using-imgui-and-implot.md): UI, themes and 2D and 3D plots.
- [Building and Toolchain](guide/building-and-toolchain.md): compilers, options, GPUs, CI and troubleshooting.
- [Profiling with Tracy](guide/profiling-with-tracy.md): the `Profiling` build and reading a capture.
- [Examples Guide](guide/examples-guide.md): the three example applications, page by page.

## Design notes

How the framework is built, and why it is shaped this way:

- [Backends](dev/backends.md): choosing a graphics backend, and which layer owns what.
- [C++ modules and the build](dev/cpp-modules.md): the module layout, `import std`, and third-party headers.
- [Dependencies](dev/dependencies.md): the fetched libraries, the ImGui stack, and how to change a pin.
- [Type names](dev/naming.md): why the graphics interface is a backend, and why the pool is `TaskSystem`.

## Process

- [Branching and releases](BRANCHING.md): branches, pull requests, and how a tag becomes a release.
- [Committing](COMMITTING.md): the message format, the types, and what one commit holds.

The API reference is not a page here: it is Doxygen output, built locally with `cmake --build build --target nodens-docs`.

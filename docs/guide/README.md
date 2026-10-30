# Nodens guide

Nodens is a C++23 framework for interactive desktop applications. These pages explain **how and why**: tutorials, architecture and design rationale. Nodens is experimental: while the major version is 0, a release that changes the API bumps the minor version. Linux is the only supported platform. The guide tracks `main`, so what changed between releases lives in the [changelog](../../CHANGELOG.md) and the [releases](https://github.com/eldritch-codex/nodens/releases).

> [!NOTE]
> **Mental model.** An `Application` is a stage manager. It owns the window, the task system and the event bus, and it runs the show one frame at a time. Your code is a set of **layers** that it calls in order: each frame, every layer updates, draws its UI and gets a chance to react to input. You write the layers; Nodens runs the loop.
>
> The guide follows that picture, in the order you will need it:
>
> 1. **Run something.** [Getting Started](./getting-started.md) builds the template.
> 2. **See the whole.** [Architecture Overview](./architecture-overview.md) shows what the application owns and what one frame does; [Application Loop](./application-loop.md) covers pacing and windowless runs.
> 3. **Meet the parts.** [Layer System](./layer-system.md) (your code), [Event System](./event-system.md) (messages) and [Concurrency and TaskSystem](./concurrency-and-tasksystem.md) (background work).
> 4. **Write your own.** [Creating a Custom Layer](./creating-a-custom-layer.md), then [Using ImGui and ImPlot](./using-imgui-and-implot.md) for the UI.
> 5. **Build and measure.** [Building and Toolchain](./building-and-toolchain.md) and [Profiling with Tracy](./profiling-with-tracy.md).
> 6. **Read working code.** [Examples Guide](./examples-guide.md).

## Where to start

| You want to | Read |
|---|---|
| Get a window on screen today | [Getting Started](./getting-started.md) |
| Understand how the pieces fit | [Architecture Overview](./architecture-overview.md) |
| Add behavior to your app | [Creating a Custom Layer](./creating-a-custom-layer.md) |
| Fix a build problem | [Building and Toolchain](./building-and-toolchain.md) |

## Pages

- **Getting started:** [Getting Started](./getting-started.md), and the [application template](https://github.com/eldritch-codex/template-nodens-app).
- **Architecture:** [Architecture Overview](./architecture-overview.md), [Application Loop](./application-loop.md), [Layer System](./layer-system.md), [Event System](./event-system.md), [Concurrency and TaskSystem](./concurrency-and-tasksystem.md).
- **Guides:** [Building and Toolchain](./building-and-toolchain.md), [Creating a Custom Layer](./creating-a-custom-layer.md), [Using ImGui and ImPlot](./using-imgui-and-implot.md), [Profiling with Tracy](./profiling-with-tracy.md).
- **Reference:** [Examples Guide](./examples-guide.md).

## This guide and the API reference

The guide explains **how and why**: tutorials, architecture and design rationale. The Doxygen API reference documents **what**: every class, function and module. It is not hosted yet; build it locally with `cmake --build build --target nodens-docs`.

The application template is a ready-made skeleton: [Nodens Application Template](https://github.com/eldritch-codex/template-nodens-app). To contribute, see [Branching and releases](../BRANCHING.md): `main` is the trunk, every change lands through a short-lived branch and a squashed pull request, and a release is an annotated tag.

## AI assistance

The maintainer uses coding agents on this guide and its pages: Claude Code and the maintainer's own agent, Armitage. Commits carry no co-author or AI attribution trailers, because the maintainer is responsible for every commit. This section is where the use of agents is disclosed.

[GitHub Repository](https://github.com/eldritch-codex/nodens) · [Releases](https://github.com/eldritch-codex/nodens/releases) · [Changelog](../../CHANGELOG.md)

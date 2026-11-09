<!-- covers: tests/, cmake/NodensDependencies.cmake, CMakeLists.txt -->
# Testing

Nodens tests its framework code with GoogleTest. Each test is one question with a known answer: run a piece of the framework with a fixed input and state the result it must give. CTest runs the test binary and CI runs CTest on every pull request.

## The decision

- One test executable per area under `tests/`, such as `task-system-tests` for the `TaskSystem`.
- GoogleTest v1.18.0, fetched and pinned in `cmake/NodensDependencies.cmake` like every other dependency.
- `gtest_discover_tests` registers each test with CTest by name, so a failure names the test and a run can filter to one.
- `ND_BUILD_TESTS` is on when Nodens is the top-level project and off when another project consumes it.

## Why

**GoogleTest has mocking.** GoogleMock is part of it. Nodens has interfaces, such as the window and the graphics backend, that code under test will need to replace. Catch2, doctest and Boost.Test have no mocking of their own, so each would need a second library for that.

**A pinned fetch matches the rest of the build.** A system GoogleTest would bring whatever version and flags the distribution chose. The test build is then the same on every machine, which is the property [Dependencies](dependencies.md) protects for the rest of the framework.

**It works with the module build.** A test file includes `<gtest/gtest.h>` and then writes `import nodens;`, the same order the examples use for standard headers. The headers go first.

## What it costs

- **A test build fetches one more repository.** Only builds with `ND_BUILD_TESTS` on.
- **Tests that need a window or a GPU are not covered yet.** The first target builds a `TaskSystem` directly, without an `Application`. A test that needs a window will have to wait for a headless path or a mock.

## Running the tests

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Run one test by name:

```bash
ctest --test-dir build -R TaskSystem.SubmitReturnsTheResult
```

## Adding a test

1. Add `TEST(Suite, Name)` to a file in `tests/`, or a new file if the area has none.
2. For a new file, add an `add_executable`, link `Nodens::Nodens` and `GTest::gtest_main`, and call `gtest_discover_tests`, as `tests/CMakeLists.txt` does for `task-system-tests`.
3. Build and run `ctest`. Name the suite after the type under test and the test after the behaviour, so a failure reads as a sentence.

```cpp
TEST(TaskSystem, SubmitReturnsTheResult)
{
    Nodens::TaskSystem tasks;
    EXPECT_EQ(tasks.Submit([] { return 6 * 7; }).get(), 42);
}
```

`EXPECT_*` records a failure and keeps going. `ASSERT_*` stops the test at the first failure.

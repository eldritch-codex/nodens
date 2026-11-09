set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(task-system-tests
    task_system_tests.cpp
)

target_compile_features(task-system-tests PUBLIC cxx_std_23)

target_link_libraries(task-system-tests PRIVATE Nodens::Nodens GTest::gtest_main)

gtest_discover_tests(task-system-tests)

add_executable(event-bus-tests
    event_bus_tests.cpp
)

target_compile_features(event-bus-tests PUBLIC cxx_std_23)

target_link_libraries(event-bus-tests PRIVATE Nodens::Nodens GTest::gtest_main)

gtest_discover_tests(event-bus-tests)

add_executable(example-headlesspulse
    HeadlessPulseApp.cpp
)

target_compile_features(example-headlesspulse PUBLIC cxx_std_23)

target_link_libraries(example-headlesspulse PRIVATE Nodens::Nodens)

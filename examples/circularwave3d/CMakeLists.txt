add_executable(example-circularwave3d
    CircularWave3DApp.cpp
    CircularWave3DLayer.cpp
)

target_sources(example-circularwave3d
    PRIVATE
        FILE_SET example_modules TYPE CXX_MODULES
        FILES
            CircularWave3DLayer.cppm
)

target_compile_features(example-circularwave3d PUBLIC cxx_std_23)

target_link_libraries(example-circularwave3d PRIVATE Nodens::Nodens)

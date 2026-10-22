add_executable(example-planetaryscan
    PlanetaryScanApp.cpp
    PlanetaryScanLayer.cpp
)

target_sources(example-planetaryscan
    PRIVATE
        FILE_SET example_modules TYPE CXX_MODULES
        FILES
            PlanetaryScanLayer.cppm
)

target_compile_features(example-planetaryscan PUBLIC cxx_std_23)

target_link_libraries(example-planetaryscan PRIVATE Nodens::Nodens)

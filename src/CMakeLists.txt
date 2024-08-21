include_directories(.)

add_subdirectory(parser)

add_executable(toyc
    toy.cpp
)

target_link_libraries(toyc parser)

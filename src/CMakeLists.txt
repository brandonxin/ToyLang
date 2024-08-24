include_directories(.)

add_subdirectory(parser)
add_subdirectory(ir)
add_subdirectory(irgen)

add_executable(toyc
    toy.cpp
)

target_link_libraries(toyc parser ir irgen)

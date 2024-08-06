add_executable(toyc
    AST.cpp
    toy.cpp
)
target_link_libraries(toyc fmt::fmt)

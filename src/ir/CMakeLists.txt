add_library(ir STATIC
    BasicBlock.cpp
    Function.cpp
    IRCompilationUnit.cpp
    Value.cpp
)

target_link_libraries(ir PUBLIC fmt::fmt)

add_library(parser STATIC AST.cpp)

target_link_libraries(parser PUBLIC fmt::fmt)

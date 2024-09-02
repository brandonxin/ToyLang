add_library(aarch64 STATIC Assembly.cpp CodeGenerator.cpp)
target_link_libraries(aarch64 PUBLIC ir)

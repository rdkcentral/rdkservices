set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-command-line-argument")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=missing-braces")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=dangling-gsl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-const-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=inconsistent-missing-override")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-parameter")

# clang Valgrind: debuginfo reader: ensure_valid failed
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4")

# x86 specific compiler settings
# ------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -arch x86_64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -arch x86_64")

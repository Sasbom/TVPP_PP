# CMakeList.txt : CMake project for TVPP_PP, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_subdirectory(mio)
add_subdirectory(miniz)
add_executable (TVPP_PP "main.cpp"  "tvp_pp/RLE.h")

if (CMAKE_VERSION VERSION_GREATER 3.20)
  set_property(TARGET TVPP_PP PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.

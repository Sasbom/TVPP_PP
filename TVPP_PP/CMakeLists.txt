# CMakeList.txt : CMake project for TVPP_PP, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_subdirectory(mio)
add_executable (TVPP_PP "main.cpp" "tvp_pp/RLE.hpp" "miniz/miniz.c" "miniz/miniz.h" "tvp_pp/zlib_util.hpp" "tvp_pp/buffer.hpp" "tvp_pp/num_util.hpp" "tvp_pp/file_util.hpp" "tvp_pp/frame_util.hpp" "tvp_pp/data.hpp" "tvp_pp/structs/FileInfo.hpp" "tvp_pp/structs/FileInfo.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.20)
  set_property(TARGET TVPP_PP PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.

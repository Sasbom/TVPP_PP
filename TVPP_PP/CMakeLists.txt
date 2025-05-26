# CMakeList.txt : CMake project for TVPP_PP, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_subdirectory(mio)

set(SOURCES
    "miniz/miniz.c"
    "miniz/miniz.h"
    "tvp_pp/RLE.hpp"
    "tvp_pp/zlib_util.hpp"
    "tvp_pp/num_util.hpp"
    "tvp_pp/file_util.hpp"
    "tvp_pp/file_util.cpp"
    "tvp_pp/frame_util.hpp"
    "tvp_pp/data.hpp"
    "tvp_pp/structs/Buffer.hpp"
    "tvp_pp/structs/Buffer.cpp"
    "tvp_pp/structs/FileInfo.hpp"
    "tvp_pp/structs/FileInfo.cpp"
    "tvp_pp/structs/ThumbInfo.hpp"
    "tvp_pp/structs/ThumbInfo.cpp" 
    "tvp_pp/structs/Buffer.hpp" 
    "tvp_pp/structs/Buffer.cpp"
    "tvp_pp/structs/Shot.hpp"
    "tvp_pp/structs/Shot.cpp"
    "tvp_pp/structs/Clip.hpp"
    "tvp_pp/structs/Clip.cpp"
    "tvp_pp/structs/Layer.hpp"
    "tvp_pp/structs/Layer.cpp"
    "tvp_pp/structs/File.hpp"
    "tvp_pp/structs/File.cpp"
    "stb/stb_image_write.h"
    "stb/stb_image_write.cpp"
) 

add_executable (TVPP_PP "main.cpp" ${SOURCES} )

if (CMAKE_VERSION VERSION_GREATER 3.20)
  set_property(TARGET TVPP_PP PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.

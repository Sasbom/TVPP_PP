cmake_minimum_required (VERSION 3.50)

project(TVPP_PP)

# mio - memory mapped file utils.
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

# On windows, export all symbols (quite dirty, but it works + I have nothing to hide.)
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library (TVPP_PP SHARED ${SOURCES})

target_include_directories(TVPP_PP
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/structs
)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.20)
  set_property(TARGET TVPP_PP PROPERTY CXX_STANDARD 23)
endif()


# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Static linking for GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "GCC detected: enabling static linking for STD")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  # set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Include TVPP_PP.
add_subdirectory ("TVPP_PP")

# Set up test application

project ("TestAPP" LANGUAGES CXX C)

add_executable(TestAPP "TVPP_PP/main.cpp")
set_property(TARGET TestAPP PROPERTY CXX_STANDARD 23)
target_link_libraries(TestAPP PRIVATE TVPP_PP)


if(WIN32)
    add_custom_command(TARGET TestAPP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TVPP_PP>
            $<TARGET_FILE_DIR:TestAPP>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(TestAPP PROPERTIES
        BUILD_RPATH "$ORIGIN"
    )

    add_custom_command(TARGET TestAPP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TVPP_PP>
            $<TARGET_FILE_DIR:TestAPP>
    )
endif()
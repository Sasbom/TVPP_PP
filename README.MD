# TVPaint Project Plus Plus - v0.2
### TVpaint and it's perils
[TVPaint](https://tvpaint.com/en), well known, well loved, is a 2D animation software that's seen widespread use in the industry.

One big dowside is that it's extremely hard to integrate into a modern pipeline. 
Because of these difficulties, I spent some time reverse engineering the file format,
and here we are, a nice library to read out the images locked away in the TvPaint file. 

This was written with the end goal of integrating it in ASWF's OpenFX, and Python through [Pybind11](https://github.com/pybind/pybind11).

It's meant to compile as a .DLL or an .so file. Cross platform compatibility between Windows and Linux was kept in mind while writing this.

**Features:**
- Read metadata about Layers / Clips / "Shots" (not really available in software).
- Reading the file itself without any direct loading of image buffers by indexing offsets in memory.
- Unpack a layer only when you need it, and keep it in cache!
- Parses number data as numbers! (seriously, the binary format stores a large amount of numeric info as textual data.)
- Request a frame out of bounds for a Layer? No problem. Pre and Post behavior (Hold, Loop, Ping Pong, None) is respected.
- Exposure frames' "Repeat Images" settings are correctly parsed.<br> 
(Note that this behavior mimics manually scrolling through the layer, 
because frames can get dropped if the "Repeat Amount" is set to a number that is larger than the available frames on play.)
- Dump everything with one simple helper function.

### Basic usage

Use this as a submodule. Clone this into your project using

`git submodule add  https://github.com/Sasbom/TVPP_PP.git`

`git submodule update --init --recursive`

This will pull in the used submodules.

The "File.hpp" header is all you need to get started. Load in a memory mapped file, and construct a File.
```c++
#include "mio/single_include/mio/mio.hpp"
#include "tvp_pp/structs/File.hpp"

int main() {
	mio::basic_mmap_source<std::uint8_t> mmap("MyEpicAnimation.tvpp"); // Load the file
	auto tvp_file = File(mmap); // Indexes offsets to read from on construction
	tvp_file.print_layers(); // print layer information
	tvp_file.dump_file_mark(); // Dump out layers, respecting mark in / mark out.
	return 0;
}
```
*A more elaborate documentation is coming (when I have time).*

---
### THINGS TO KEEP IN MIND WHEN USING THIS
This project is still in it's early stages. It might be unstable and clunky to use.<br>
A cleanup is required but the main functionality is there.

The image buffers are top-to-bottom, which might not align with some standards.

**NOTE THAT FOR NOW, ONLY COMPRESSED TVPAINT FILES CAN BE PROCESSED!!**
<br>This is how we save our files, uncompressed parsing is very similar but still needs to be implemented properly.

---
### Integrating this in your top CMakeLists

The following snipped should get you up and running.
```cmake
# Set up project.
project (${PROJECT_NAME} LANGUAGES CXX C)
add_executable(${PROJECT_NAME} "path-to-entrypoint-definition.cpp")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

# Include the TVPP_PP subdirectory
add_subdirectory ("TVPP_PP")

# Link.
target_link_libraries(${PROJECT_NAME} PRIVATE TVPP_PP)

# Ensure presence of library in build dir of head project and copy over the .DLL/.so to be relative to the 
exported file.
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TVPP_PP>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # On linux, set RPATH to folder it gets exported. (needed for relative .so files)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TVPP_PP>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
```

---
### TODO:
- [ ] Nuke OpenFX plugin
  - [ ] OpenFX plugin for other hosts (Big maybe)
- [ ] Write python bindings / hints.
- [ ] Documentation
- [ ] Document findings in the file format (it's more interesting than you think!)
- [ ] Add support for uncompressed files
- [ ] Proper namespaces?
- [ ] Perform major cleanup, especially concerning printed content.
---
### Acknowledgements:
I want to thank the following authors for making useful libraries that helped make this possible.
Check them out at your leisure!

- [kguyaux](https://github.com/kguyaux) | Proof of concept
<br>A former colleague who proved this was possible. They made a proof of concept in python. [LINK](https://github.com/kguyaux/tvpaint_export)

- [vimpunk](https://github.com/vimpunk) | mio
<br>A fantastic libary for cross platform memory mapped files. [LINK](https://github.com/vimpunk/mio)

- [richgel999](https://github.com/richgel999) | miniz
<br>ZLib dropin C library, making managing decompression in project a whole lot easier. [LINK](https://github.com/richgel999/miniz)

- [nemtrif](https://github.com/nemtrif) | utfcpp
<br>Saved me major headaches dealing with UTF standards. [LINK](https://github.com/nemtrif/utfcpp)

- [nothings](https://github.com/nothings) | stb
<br>Allowed me some quick image exports. Well known, well loved. [LINK](https://github.com/nothings/stb)


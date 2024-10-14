cmake_minimum_required(VERSION 3.10)
project(libbloom VERSION 1.0 LANGUAGES C)

# Set the output directory for libraries and executables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add source files for the library
set(SRC_FILES
    src/mmh3.c
    src/bloom.c
    src/countingbloom.c
    src/timedecay.c
    src/cuckoo.c
)

# Create a static library (.a)
#add_library(bloom_static STATIC ${SRC_FILES})
#set_target_properties(bloom_static PROPERTIES OUTPUT_NAME "bloom-static")

# Create a shared library (.so)
add_library(bloom_shared SHARED ${SRC_FILES})
set_target_properties(bloom_shared PROPERTIES OUTPUT_NAME "bloom")

# Include directories for the library headers
#target_include_directories(bloom_static PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(bloom_shared PUBLIC ${PROJECT_SOURCE_DIR}/src)

# Link against the math library for both shared and static versions
#target_link_libraries(bloom_static PUBLIC m)
target_link_libraries(bloom_shared PUBLIC m)

# Optionally add an example/test program
add_executable(test_bloom tests/main.c)

# Link the example program with the shared library
target_link_libraries(test_bloom PRIVATE bloom_shared)

# Install rules
install(TARGETS bloom_shared # bloom_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES
    src/bloom.h
    src/mmh3.h
    src/timedecay.h
    src/countingbloom.h
    src/cuckoo.h
    DESTINATION include/bloom)
install(CODE "execute_process(COMMAND ldconfig)")

# Enable testing
enable_testing()
add_test(NAME TestBloom COMMAND bin/test_bloom)

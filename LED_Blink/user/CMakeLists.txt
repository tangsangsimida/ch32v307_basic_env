# User application code

# Auto-collect user sources
file(GLOB USER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Create OBJECT library
add_library(user_objects OBJECT ${USER_SRC})

# Include directories: own dir + peripheral library headers
target_include_directories(user_objects PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to peripheral library for header access
target_link_libraries(user_objects PRIVATE ch32v30x_peripheral)

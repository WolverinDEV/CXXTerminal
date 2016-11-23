cmake_minimum_required(VERSION 3.2.2)
project(CXXTerminal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#link pthread
link_libraries(pthread)

# Find library headers
file(GLOB HEADERS include/*.h)
# Find source files
file(GLOB SOURCES src/*.cpp)
# Find source files
file(GLOB TEST test/*)


# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION lib/)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

#Create test
add_executable(${PROJECT_NAME}Test ${TEST} ${HEADERS} ${SOURCES})
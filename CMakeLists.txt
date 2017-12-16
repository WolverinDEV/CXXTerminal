cmake_minimum_required(VERSION 3.6.0)
project(CXXTerminal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#add_definitions(-DTERMINAL_NON_BLOCK)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

#link pthread
link_libraries(pthread)

# Find library headers
file(GLOB HEADERS include/*.h)
# Find source files
file(GLOB_RECURSE SOURCES src/*.cpp)
# Find source files
file(GLOB TEST test/*)

message("Src: ${HEADERS} - ${SOURCES} - ${TEST}")

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(${PROJECT_NAME}Static STATIC ${SOURCES} ${HEADERS})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${PROJECT_NAME}Static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

#Create test
add_executable(${PROJECT_NAME}Test ${TEST} ${HEADERS} ${SOURCES})
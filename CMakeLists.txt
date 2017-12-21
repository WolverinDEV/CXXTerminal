cmake_minimum_required(VERSION 3.6.0)
project(CXXTerminal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#add_definitions(-DTERMINAL_NON_BLOCK)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
add_definitions(-DUSE_LIBEVENT)

#link pthread
link_libraries(pthread)

set(SRC_FILES
		src/utils/CString.cpp
		src/utils/Grapth.cpp

		src/ArgumentParser.cpp
		src/TabCompleter.cpp
		src/Terminal.cpp
		src/TerminalAnsi.cpp
		src/TerminalIO.cpp)

# Find library headers
file(GLOB HEADERS include/*.h)
# Find source files
file(GLOB TEST test/*)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} event)

add_library(${PROJECT_NAME}Static STATIC ${SRC_FILES} ${HEADERS})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${PROJECT_NAME}Static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

#Create test
add_executable(${PROJECT_NAME}Test ${TEST} ${HEADERS} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}Test event)
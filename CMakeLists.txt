cmake_minimum_required(VERSION 3.6.0)
project(CXXTerminal)

#set(CMAKE_CXX_COMPILER "arm-linux-gnueabi-g++")
#set(CMAKE_CXX_COMPILER_AR "arm-linux-gnueabi-g++")
#set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	#link pthread
	link_libraries(pthread)
else()
	link_libraries(Ws2_32)
endif()
#add_definitions(-DTERMINAL_NON_BLOCK)

set(SRC_FILES
	src/utils/CString.cpp
	src/utils/Grapth.cpp

	src/ArgumentParser.cpp
	src/TabCompleter.cpp
	src/Terminal.cpp
	src/TerminalAnsi.cpp
	src/TerminalIO.cpp
)

# Find library headers
file(GLOB HEADERS include/*.h)
# Find source files
file(GLOB TEST test/*)

option(ENABLE_LIBEVENT "Enable/disable libevent" ON)
option(NO_EMBETTED_LIBEVENT "Enable/disable internal libevent library" OFF)
if(ENABLE_LIBEVENT)
	if(NOT NO_EMBETTED_LIBEVENT)
		include_directories(libraries/event/include)
		include_directories(libraries/event/build/include)
		if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libraries/event/build/lib/libevent.a")
			message(FATAL_ERROR "If you want to use libevent than please build libevent first! (libraries/event)")
		endif()
		set(EVENT_LIBRARIES "${CMAKE_SOURCE_DIR}/libraries/event/build/lib/libevent.a" "${CMAKE_SOURCE_DIR}/libraries/event/build/lib/libevent_pthreads.a")
	else()
		find_package(Libevent REQUIRED)
		# include(${CMAKE_MODULE_PATH}/libraries_wolverin_lap.cmake)

		include_directories(${LIBEVENT_INCLUDE_DIRS})
		set(EVENT_LIBRARIES ${LIBEVENT_STATIC_LIBRARIES})
	endif()
    add_definitions(-DUSE_LIBEVENT)
endif()

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADERS})
if(ENABLE_LIBEVENT)
    target_link_libraries(${PROJECT_NAME} ${EVENT_LIBRARIES})
endif()
add_library(${PROJECT_NAME}Static STATIC ${SRC_FILES} ${HEADERS})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${PROJECT_NAME}Static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

#Create test
add_executable(${PROJECT_NAME}Test ${TEST} ${HEADERS} ${SRC_FILES})
if(ENABLE_LIBEVENT)
	message("Linking libevent")
    target_link_libraries(${PROJECT_NAME}Test ${EVENT_LIBRARIES})
endif()

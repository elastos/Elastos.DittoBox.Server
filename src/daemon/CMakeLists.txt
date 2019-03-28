project(elaoc-agentd C)

include(CarrierDefaults)
include(CheckIncludeFile)

set(SRC
    config.c
    pfd.c)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread)
endif()

include_directories(
    ${CARRIER_SDK_PATH}/include
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_SDK_PATH}/lib
    ${CARRIER_INT_DIST_DIR}/lib)

add_definitions(-DLIBCONFIG_STATIC)

check_include_file(getopt.h HAVE_GETOPT_H)
if(HAVE_GETOPT_H)
    add_definitions(-DHAVE_GETOPT_H=1)
endif()

set(LIBS
    elacarrier
    elasession
    crystal
    config)

add_executable(elaoc-agentd ${SRC})
target_link_libraries(elaoc-agentd ${LIBS})

install(TARGETS elaoc-agentd
    RUNTIME DESTINATION "usr/bin"
    ARCHIVE DESTINATION "usr/lib"
    LIBRARY DESTINATION "usr/lib")

install(PROGRAMS ${CARRIER_SDK_PATH}/lib/libelacarrier${CMAKE_SHARED_LIBRARY_SUFFIX}
    DESTINATION "usr/lib")

install(PROGRAMS ${CARRIER_SDK_PATH}/lib/libelasession${CMAKE_SHARED_LIBRARY_SUFFIX}
    DESTINATION "usr/lib")

install(PROGRAMS elaoc-agent.sh
    DESTINATION "usr/bin")

cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)
project(elaoc-agentd)

# set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CarrierDefaults)
include(CarrierPackages)
include(AddSubModule)

if("${CARRIER_SDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "CARRIER_SDK_PATH variable not set!")
endif()

add_subdirectory(deps)
add_subdirectory(src)

# Alias target for package. But it fix issue about making package
# inrelevant to udpatation of cmake scripts.
#add_custom_target(dist
#    COMMAND ${CMAKE_MAKE_PROGRAM} package
#    COMMENT "Package Carrier Distrubtion")

install(FILES config/elaoc-agent.conf
    DESTINATION "etc/elaoc")

install(FILES scripts/elaoc-agentd.service
    DESTINATION "lib/systemd/system")

install(PROGRAMS scripts/elaoc-agentd.sh
    DESTINATION "etc/init.d"
    RENAME "elaoc-agentd")

install(DIRECTORY
    DESTINATION "var/lib/elaoc-agentd")

# make dist package
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_custom_target(dist
        COMMAND ${CMAKE_MAKE_PROGRAM} package
        COMMENT "Package Elastos Carrier Agent for ownCloud package Distrubtion")
endif()

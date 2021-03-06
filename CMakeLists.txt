cmake_minimum_required(VERSION 3.2)
project(usercomponent)

# point cmake to my own custom installation directory that's not a system dir
# this is where BDSIMConfig.cmake exists
set(CMAKE_PREFIX_PATH /Users/eliottramoisiaux/reps/clion-builds/bdsim-debug-install/lib/cmake/bdsim)

# find the package and set up variables
find_package(BDSIM REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# GDML (on by default)
option( USE_GDML "Include GDML support."  ON )
if(USE_GDML)
    add_definitions("-DUSE_GDML")
    message(STATUS "GDML support ON")
endif()

# add headers from BDSIM
include_directories(${BDSIM_INCLUDE_DIR}/bdsim)

# add headers for this project
include_directories(include)

# sources - put all compiled sources in a library
file(GLOB SOURCES "source/*.cc")
add_library(udipole SHARED ${SOURCES})
# link to bdsim parser (gmad) then bdsim (order specific)
target_link_libraries(udipole gmad bdsim)

# make a program and link to udipole source code library
add_executable(bdsimuser usercomponent.cc)
# here we don't link to gmad and bdsim again - just udipole library as that
# has the symbols we need already
target_link_libraries(bdsimuser udipole)
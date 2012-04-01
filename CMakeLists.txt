cmake_minimum_required (VERSION 2.6)
project (waveformgen)
add_executable(waveformgen waveformgen.c main.c)
set(CMAKE_C_FLAGS "-std=c99") 

find_library (SNDFILE sndfile)
find_library (GD gd)
find_path (GD_INCLUDE_DIR gd.h)
find_path (SNDFILE_INCLUDE_DIR sndfile.h)

if(GD)
    message(STATUS "Found libGD: " ${GD})
endif(GD)

if(NOT GD)
    message(SEND_ERROR "Cannot find libGD")
    set(HAS_ERROR true)
endif(NOT GD)

if(SNDFILE)
    message(STATUS "Found libsndfile: " ${SNDFILE})
endif(SNDFILE)

if(NOT SNDFILE)
    message(SEND_ERROR "Cannot find libsndfile")
    set(HAS_ERROR true)
endif(NOT SNDFILE)

if(NOT HAS_ERROR)
    include_directories(
        ${GD_INCLUDE_DIR}
        ${SNDFILE_INCLUDE_DIR}
    )

    target_link_libraries(waveformgen ${SNDFILE} ${GD})
endif(NOT HAS_ERROR)

install (TARGETS waveformgen RUNTIME DESTINATION bin)
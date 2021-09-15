cmake_minimum_required(VERSION 3.10)
project(ParamatricSimulation)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -march=native -Wno-int-in-bool-context")

macro(find_libraries)
    find_package(MPI REQUIRED)
    include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

    find_package(Boost 1.65.0 COMPONENTS serialization mpi graph REQUIRED)

    if (Boost_FOUND)
        message(STATUS "Using boost.")
        message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
        message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
        message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    else()
        message(FATAL_ERROR "Can not found boost")
    endif ()

    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    include("cmake/FindGlog.cmake")
    if (NOT GLOG_FOUND)
        message(FATAL_ERROR "glog not found, please install the glog library")
    else ()
        include_directories(SYSTEM ${GLOG_INCLUDE_DIRS})
    endif ()

    include("cmake/FindGFlags.cmake")

    if (NOT GFLAGS_FOUND)
        message(FATAL_ERROR "gflags not found, please install the gflags library")
    else ()
        include_directories(SYSTEM ${GFLAGS_INCLUDE_DIRS})
    endif ()
endmacro(find_libraries)

find_libraries()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(thirdparty/eigen)

add_executable(her her/her.cc her/flags.cc)
target_link_libraries(her ${MPI_CXX_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} -lboost_mpi -lboost_graph -lboost_serialization)
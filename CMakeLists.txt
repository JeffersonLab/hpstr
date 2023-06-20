# minimum cmake version
cmake_minimum_required(VERSION 3.18)

project(hpstr)

#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_C_STANDARD 14)
#set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)       # set C++ standard we use within ldmx-sw

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# enable in source build since default is not writable from a non-root account
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE)
endif()

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# find the LCIO directory
if (LCIO_DIR)
    set(LCIO_INCLUDE_DIR "${LCIO_DIR}/include")

    #TEMPORARY: if MacOs LCIO standard compilation creates dynamic libs. Correct for that. 
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(LCIO_LIBRARY "${LCIO_DIR}/lib/liblcio.dylib")
    else()
        set(LCIO_LIBRARY "${LCIO_DIR}/lib/liblcio.so")
    endif()	       

    if (NOT EXISTS "${LCIO_DIR}")
        message(FATAL_ERROR "Unable to find LCIO library")
    endif()
    message(STATUS "LCIO dir set to: ${LCIO_DIR}")

    # The following are not necessary - should clean up
    message(STATUS "LCIO include dir set to: ${LCIO_INCLUDE_DIR}")
    message(STATUS "LCIO library set to: ${LCIO_LIBRARY}")
endif()
find_package(LCIO REQUIRED)

# find Python 3
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python executable found at: ${Python3_EXECUTABLE}")
message(STATUS "Python lib found at: ${Python3_LIBRARIES}")
message(STATUS "Python include dir found at: ${Python3_INCLUDE_DIRS}")

# find ROOT installation
find_package(ROOT REQUIRED COMPONENTS Core RIO Geom Eve Gui)
message(STATUS "ROOT found at: ${ROOT_DIR}")



find_package(Eigen3 CONFIG REQUIRED)

# since ACTS changes frequently enough, we build it as a submodule of Tracking
#   here we 'set' some CMake variables that we would have passed to 'cmake' on
#   the command line with '-D' if we were building ACTS directly.
set(ACTS_BUILD_PLUGIN_IDENTIFICATION ON) #This turns on the identiers
set(ACTS_BUILD_EXAMPLES OFF)     # don't waste time building examples
set(ACTS_USE_SYSTEM_BOOST OFF)  # get boost version with acts

add_subdirectory(acts)           # now build acts
#add_subdirectory(acts-dd4hep)
# luckily for us, adding ACTS as a subdirectory produces the same CMake targets
#   as find_package, so we don't need to do anything else from here on out





# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# import macro for declaring external dependencies
include(MacroExtDeps)

set(MODULES event analysis processing processors)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module: ${module}")
  add_subdirectory(${module})
endforeach()

# configure and generate documentation using doxygen
#option(INSTALL_DOC "Set to ON to generate documentation using doxygen" OFF)
#message(STATUS "Doxygen documentation: ${INSTALL_DOC}")
#if(INSTALL_DOC)

    # message that documentation is off for this build
    #    message(STATUS "Doxygen documentation will be generated")

    # find doxygen
    #    find_program(DOXYGEN_EXECUTABLE doxygen ${PATH})
    #    if(DOXYGEN_EXECUTABLE-NOTFOUND)
    #        message(FATAL_ERROR "The doxygen executable was not found")
    #    endif()

    # find dot
    #find_program(DOT_EXECUTABLE dot ${PATH})
    #if(DOT_EXECUTABLE-NOTFOUND)
    #    message(FATAL_ERROR "The dot executable was not found.")
    #endif()
    
    # configure doxygen file
    #    configure_file(${PROJECT_SOURCE_DIR}/docs/doxygen.conf.in ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

    # generate the documentation
    #    install(CODE "execute_process(COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")

    # documentation generation target 
    #    add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

    #endif()

# configure and install setup script
configure_file(hpstr-env.sh.in ${CMAKE_INSTALL_PREFIX}/bin/hpstr-env.sh)

# install JSON analysis config files
#install(DIRECTORY analysis 
#        DESTINATION ${CMAKE_INSTALL_PREFIX}
#        FILES_MATCHING PATTERN "*.json"
#        PATTERN "src" EXCLUDE
#        PATTERN "include" EXCLUDE)

# install python files
install(DIRECTORY plotUtils DESTINATION lib/python)
install(FILES processors/config/baseConfig.py DESTINATION lib/python)

# info message about install prefix
message(STATUS "hpstr will be installed to: ${CMAKE_INSTALL_PREFIX}")


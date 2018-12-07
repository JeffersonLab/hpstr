# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# find the LCIO directory
if (LCIO_DIR)
    set(LCIO_INCLUDE_DIR "${LCIO_DIR}/include")
    set(LCIO_LIBRARY "${LCIO_DIR}/lib/liblcio.so")
    if (NOT EXISTS "${LCIO_DIR}")
        message(FATAL_ERROR "Unable to find LCIO library")
    endif()
    message(STATUS "LCIO dir set to: ${LCIO_DIR}")
    message(STATUS "LCIO include dir set to: ${LCIO_INCLUDE_DIR}")
    message(STATUS "LCIO library set to: ${LCIO_LIBRARY}")
endif()
find_package(LCIO REQUIRED)

# find Python installation
find_package(PythonLibs REQUIRED)
message(STATUS "Python lib found at: ${PYTHON_LIBRARIES}")
message(STATUS "Python include dir found at: ${PYTHON_INCLUDE_DIRS}")
get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)

# find ROOT installation
find_package(ROOT REQUIRED COMPONENTS Core RIO PyROOT Geom Eve Gui)
message(STATUS "ROOT found at: ${ROOT_DIR}")

# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# import macro for declaring external dependencies
include(MacroExtDeps)

# declare list of modules with correct dependency order
#set(MODULES Event Framework DetDescr Tools EventProc SimCore SimPlugins Biasing SimApplication Configuration Detectors Ecal EventDisplay)

set(MODULES event processing processors)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module: ${module}")
  add_subdirectory(${module})
endforeach()

# install python init file for top-level LDMX module
#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py "# python package")
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION lib/python/LDMX)

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

# option for dumping full CMake env
option(DUMP_CMAKE_VARIABLES OFF)
if(DUMP_CMAKE_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif()

# info message about install prefix
message(STATUS "hpstr will be installed to: ${CMAKE_INSTALL_PREFIX}")

# Install script for directory: /home/alic/src/hpstr/analysis

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/alic/src/hpstr/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/alic/src/hpstr/install/lib/libanalysis.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/alic/src/hpstr/install/lib" TYPE SHARED_LIBRARY FILES "/home/alic/src/hpstr/build/analysis/libanalysis.so")
  if(EXISTS "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so"
         OLD_RPATH "/home/alic/src/root-6.18.04/buildV61804/lib:/home/alic/src/hpstr/build/event:/home/alic/work/hps/hpsgit/LCIO/install/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/alic/src/hpstr/install/lib/libanalysis.so")
    endif()
  endif()
endif()


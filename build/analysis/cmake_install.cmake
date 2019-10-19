# Install script for directory: /u/re/alspellm/work/src/hpstr/analysis

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/u/re/alspellm/work/src/hpstr/install")
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
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/u/re/alspellm/work/src/hpstr/install/lib" TYPE SHARED_LIBRARY FILES "/u/re/alspellm/work/src/hpstr/build/analysis/libanalysis.so")
  if(EXISTS "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so"
         OLD_RPATH "/nfs/slac/g/hps3/users/bravo/src/root/buildV61204/lib:/u/re/alspellm/work/src/hpstr/build/event:/nfs/slac/g/hps3/users/alspellm/src/LCIO/install/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/opt/rh/devtoolset-8/root/usr/bin/strip" "$ENV{DESTDIR}/u/re/alspellm/work/src/hpstr/install/lib/libanalysis.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()


find_path(HPSEvioReader_INCLUDE_DIR HPSEvioReader.h ${HPSEvioReader_DIR}/include)

find_library(HPSEvioReader_LIBRARY HPSEvioReader ${HPSEvioReader_DIR}/lib)


set(HPSEvioReader_LIBRARIES ${HPSEvioReader_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args( HPSEvioReader DEFAULT_MSG HPSEvioReader_LIBRARIES HPSEvioReader_INCLUDE_DIR )

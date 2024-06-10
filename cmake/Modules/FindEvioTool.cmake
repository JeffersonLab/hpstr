find_path(EvioTool_INCLUDE_DIR EvioTool.h ${EvioTool_DIR}/include)

find_library(EvioTool_LIBRARY EvioTool ${EvioTool_DIR}/lib)


set(EvioTool_LIBRARIES ${EvioTool_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args( EvioTool DEFAULT_MSG EvioTool_LIBRARIES EvioTool_INCLUDE_DIR )

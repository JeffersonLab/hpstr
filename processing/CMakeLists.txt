
# Declare processing module
module( 
    NAME processing
    EXTRA_SOURCES processingDict.cxx
    DEPENDENCIES event 
    EXTERNAL_DEPENDENCIES ROOT Python LCIO
)

root_generate_dictionary(processingDict ${processing_INCLUDE_DIR}/processingDef.h MODULE ${PROJECT_NAME} LINKDEF ${processing_INCLUDE_DIR}/processingLinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

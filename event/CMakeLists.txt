
# Declare processing module
module( 
    NAME event 
    EXTRA_SOURCES EventDict.cxx
    EXTERNAL_DEPENDENCIES ROOT LCIO 
)

root_generate_dictionary(EventDict ${event_INCLUDE_DIR}/EventDef.h MODULE ${PROJECT_NAME} LINKDEF ${event_INCLUDE_DIR}/EventLinkDef.h)

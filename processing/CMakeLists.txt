
# Declare processing module
module( 
    NAME processing 
    EXECUTABLES src/hpstr.cxx
    DEPENDENCIES event 
    EXTERNAL_DEPENDENCIES ROOT Python LCIO Boost
)

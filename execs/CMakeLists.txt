
# Declare processing module
module( 
  NAME hpstrexecs
  EXECUTABLES src/hpstr.cxx
  DEPENDENCIES event processing 
  EXTERNAL_DEPENDENCIES ROOT Python LCIO
)

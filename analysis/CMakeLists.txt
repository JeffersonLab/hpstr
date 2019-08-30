
# Declare analysis module
module(
    NAME analysis
    EXECUTABLES ./src/runAnalysis.cxx
    DEPENDENCIES event
    EXTERNAL_DEPENDENCIES ROOT 
)

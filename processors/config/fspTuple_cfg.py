import HpstrConf
import sys

import baseConfig as base
from baseConfig import bfield

base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
    help="Which tracking to use to make plots", metavar="tracking", default="KF")
base.parser.add_argument("-s", "--truthHits", type=int, dest="truthHits",
        help="Get svt truth hits: 1=yes", metavar="truthHits", default=1)
base.parser.add_argument("-r", "--rawHits", type=int, dest="rawHits",
        help="Keep raw svt hits: 1=yes", metavar="rawHits", default=1)

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename
root_file = options.outFilename

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()

#p.max_events = 1000
p.run_mode = 0
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
header  = HpstrConf.Processor('header', 'EventProcessor')
fsp = HpstrConf.Processor("fps",'FinalStateParticleProcessor')

###############################
#   Processor Configuration   #
###############################
#Event
header.parameters["debug"] = 0
header.parameters["headCollRoot"] = "EventHeader"
header.parameters["trigCollLcio"] = "TriggerBank"
header.parameters["rfCollLcio"]   = "RFHits"
header.parameters["vtpCollLcio"]  = "VTPBank"
header.parameters["vtpCollRoot"]  = "VTPBank"
header.parameters["tsCollLcio"]   = "TSBank"
header.parameters["tsCollRoot"]   = "TSBank"

#FinalStateParticleProcessor
fsp.parameters["debug"] = 0 
fsp.parameters["fspCollLcio"] = "FinalStateParticles_KF" 
fsp.parameters["fspCollRoot"] = "FinalStateParticles_KF"
fsp.parameters["kinkRelCollLcio"] = ""
fsp.parameters["trkRelCollLcio"] = "KFTrackDataRelations"
if(options.rawHits==1):
    fsp.parameters["trkhitCollRoot"] = "fspOnTrackHits"
    fsp.parameters["rawhitCollRoot"] = "fspOnTrackRawHits"
    fsp.parameters["hitFitsCollLcio"] = "SVTFittedRawTrackerHits"
else:
    fsp.parameters["trkhitCollRoot"] = "fspOnTrackHits"
    fsp.parameters["rawhitCollRoot"] = ""
    fsp.parameters["hitFitsCollLcio"] = ""
    
p.sequence = [header,fsp]

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

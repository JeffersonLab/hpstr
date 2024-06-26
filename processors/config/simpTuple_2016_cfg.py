import HpstrConf
import sys
import baseConfig as base
from baseConfig import bfield

base.parser.add_argument("-TS", "--trackstate", type=str, dest="trackstate",
                         help="Specify Track State | 'AtECal', 'AtTarget'. Default is origin ",  metavar="trackstate", default="AtTarget")

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename
root_file = options.outFilename

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()

# p.max_events = 1000
p.run_mode = 0
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
header = HpstrConf.Processor('header', 'EventProcessor')
vtx = HpstrConf.Processor('vtx', 'VertexProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')

###############################
#   Processor Configuration   #
###############################
# Event
header.parameters["debug"] = 0
header.parameters["headCollRoot"] = "EventHeader"
header.parameters["trigCollLcio"] = "TriggerBank"
header.parameters["rfCollLcio"] = "RFHits"
header.parameters["vtpCollLcio"] = "VTPBank"
header.parameters["vtpCollRoot"] = "VTPBank"
header.parameters["tsCollLcio"] = "TSBank"
header.parameters["tsCollRoot"] = "TSBank"

# Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["vtxCollRoot"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["partCollRoot"] = 'ParticlesOnUVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
vtx.parameters["trkhitCollRoot"] = ''
vtx.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
vtx.parameters["rawhitCollRoot"] = ''
vtx.parameters["trackStateLocation"] = options.trackstate 
if options.trackstate == "":
        vtx.parameters["bfield"] = bfield[str(options.year)]
vtx.parameters["mcPartRelLcio"] = 'SVTTrueHitRelations'

# MCParticle
mcpart.parameters["debug"] = 0
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

sequence = [header, vtx]

# If MC, get MCParticles
if (not options.isData):
    sequence.append(mcpart)

p.sequence = sequence

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

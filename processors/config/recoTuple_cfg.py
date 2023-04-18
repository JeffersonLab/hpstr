import HpstrConf
import sys

import baseConfig as base
from baseConfig import bfield

base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
                         help="Which tracking to use to make plots", metavar="tracking", default="KF")
base.parser.add_argument("-s", "--truthHits", type=int, dest="truthHits",
                         help="Get svt truth hits: 1=yes", metavar="truthHits", default=0)
base.parser.add_argument("-r", "--rawHits", type=int, dest="rawHits",
        help="Keep raw svt hits: 1=yes", metavar="rawHits", default=0)

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
track = HpstrConf.Processor('track', 'TrackingProcessor')
trackgbl = HpstrConf.Processor('trackgbl', 'TrackingProcessor')
trackrefitgbl = HpstrConf.Processor('trackrefitgbl', 'TrackingProcessor')
svthits = HpstrConf.Processor('svthitskf', 'Tracker2DHitProcessor')
svthitsgbl = HpstrConf.Processor('svthitsgbl', 'Tracker3DHitProcessor')
rawsvt = HpstrConf.Processor('rawsvt', 'SvtRawDataProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
vtx = HpstrConf.Processor('vtx', 'VertexProcessor')
cvtx = HpstrConf.Processor('cvtx', 'VertexProcessor')
vtxgbl = HpstrConf.Processor('vtxgbl', 'VertexProcessor')
cvtxgbl = HpstrConf.Processor('cvtxgbl', 'VertexProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')
fsp = HpstrConf.Processor('fps', 'FinalStateParticleProcessor')

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

# SvtRawData
rawsvt.parameters["debug"] = 0
rawsvt.parameters["hitCollLcio"] = 'SVTRawTrackerHits'
rawsvt.parameters["hitfitCollLcio"] = 'SVTFittedRawTrackerHits'
rawsvt.parameters["hitCollRoot"] = 'SVTRawTrackerHits'

# Tracker3DHits
svthits.parameters["debug"] = 0
svthits.parameters["hitCollLcio"] = 'RotatedHelicalTrackHits'
svthits.parameters["hitCollRoot"] = 'RotatedHelicalTrackHits'


# Tracking
track.parameters["debug"] = 0
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
track.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack_KF'

track.parameters["bfield"] = bfield[str(options.year)]

trackgbl.parameters["debug"] = 0
trackgbl.parameters["trkCollLcio"] = 'GBLTracks'
trackgbl.parameters["trkCollRoot"] = 'GBLTracks'
trackgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
trackgbl.parameters["trkRelCollLcio"] = 'TrackDataRelations'
trackgbl.parameters["trkhitCollRoot"] = 'RotatedHelicalOnTrackHits'
trackgbl.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
trackgbl.parameters["rawhitCollRoot"] = ''#'SVTRawHitsOnTrack'
trackgbl.parameters["bfield"] = bfield[str(options.year)]

# if (not options.isData):
#    trackgbl.parameters["truthTrackCollLcio"] = 'GBLTracksToTruthTrackRelations'
#    trackgbl.parameters["truthTrackCollRoot"] = 'Truth_GBLTracks'

# ECalData
ecal.parameters["debug"] = 0
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

# Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["vtxCollRoot"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["partCollRoot"] = 'ParticlesOnUVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

cvtx.parameters["debug"] = 0
cvtx.parameters["vtxCollLcio"] = 'TargetConstrainedV0Vertices_KF'
cvtx.parameters["vtxCollRoot"] = 'TargetConstrainedV0Vertices_KF'
cvtx.parameters["partCollRoot"] = 'ParticlesOnCVertices_KF'
cvtx.parameters["kinkRelCollLcio"] = ''
cvtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'


vtxgbl.parameters["debug"] = 0
vtxgbl.parameters["vtxCollLcio"] = 'UnconstrainedV0Vertices'
vtxgbl.parameters["vtxCollRoot"] = 'UnconstrainedV0Vertices'
vtxgbl.parameters["partCollRoot"] = 'ParticlesOnUVertices'
vtxgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
vtxgbl.parameters["trkRelCollLcio"] = 'TrackDataRelations'

cvtxgbl.parameters["debug"] = 0
cvtxgbl.parameters["vtxCollLcio"] = 'TargetConstrainedV0Vertices'
cvtxgbl.parameters["vtxCollRoot"] = 'TargetConstrainedV0Vertices'
cvtxgbl.parameters["partCollRoot"] = 'ParticlesOnCVertices'
cvtxgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
cvtxgbl.parameters["trkRelCollLcio"] = 'TrackDataRelations'

# MCParticle
mcpart.parameters["debug"] = 0
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

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

# Sequence which the processors will run.

if (options.tracking == "KF"):
    print("KF")
    sequence = [header, vtx, cvtx, ecal, track]
    # Get KF svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthits)
elif (options.tracking == "GBL"):
    print("GBL")
    sequence = [header, vtxgbl, cvtxgbl, ecal, trackgbl]
    # Get GBL svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthitsgbl)
elif (options.tracking == "BOTH"):
    sequence = [header, vtxgbl, cvtxgbl, trackgbl, vtx, cvtx, ecal, track]
    # Get KF and GBL svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthits)
        sequence.append(svthitsgbl)
else:
    print("ERROR::Need to specify which tracks KF, GBL, or BOTH")

if options.isData == -1:
    print("Please specficy if this is Data or not via option -t")
if (not options.isData):
    sequence.append(mcpart)
sequence.append(fsp)

p.sequence = sequence

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

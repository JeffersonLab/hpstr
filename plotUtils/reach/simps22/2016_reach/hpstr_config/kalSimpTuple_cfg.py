import HpstrConf
import sys

import baseConfig as base
from baseConfig import bfield

base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
                         help="Which tracking to use to make plots", metavar="tracking", default="GBL")
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
ecal_gbl = HpstrConf.Processor('ecalgbl', 'ECalDataProcessor')
ecal_kf = HpstrConf.Processor('ecalkf', 'ECalDataProcessor')
vtx = HpstrConf.Processor('vtx', 'VertexProcessor')
vtxgbl = HpstrConf.Processor('vtxgbl', 'VertexProcessor')
cvtxgbl = HpstrConf.Processor('cvtxgbl', 'VertexProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')

###############################
#   Processor Configuration   #
###############################
#Event
header.parameters["debug"] = 0
header.parameters["headCollRoot"] = "EventHeader"
header.parameters["trigCollLcio"] = "TriggerBank"
header.parameters["rfCollLcio"] = "RFHits"
header.parameters["vtpCollLcio"] = "VTPBank"
header.parameters["vtpCollRoot"] = "VTPBank"
header.parameters["tsCollLcio"] = "TSBank"
header.parameters["tsCollRoot"] = "TSBank"

#SvtRawData
rawsvt.parameters["debug"] = 0
rawsvt.parameters["hitCollLcio"] = 'SVTRawTrackerHits'
rawsvt.parameters["hitfitCollLcio"] = 'SVTFittedRawTrackerHits'
rawsvt.parameters["hitCollRoot"] = 'SVTRawTrackerHits'

#Tracker2DHits
svthits.parameters["debug"] = 0
svthits.parameters["hitCollLcio"] = 'StripClusterer_SiTrackerHitStrip1D'
svthits.parameters["hitCollRoot"] = 'SiClusters'
svthits.parameters["mcPartRelLcio"] = 'SVTTrueHitRelations'

#Tracker3DHits
svthitsgbl.parameters["hitCollLcio"] = 'RotatedHelicalTrackHits'
svthitsgbl.parameters["hitCollRoot"] = 'RotatedHelicalTrackHits'
svthitsgbl.parameters["mcPartRelLcio"] = 'RotatedHelicalTrackMCRelations'

#Tracking
track.parameters["debug"] = 0
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'

#Only for detail studies
#LT uncomment
track.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack_KF'

#LT uncommented
#if (not options.isData):
#    track.parameters["truthTrackCollLcio"] = 'KalmanFullTracksToTruthTrackRelations'
#    track.parameters["truthTrackCollRoot"] = 'Truth_KFTracks'

#LT check if we need the b field or not -- version of HPS java
# for Jess's files need to give it b-field

track.parameters["bfield"] = bfield[str(options.year)]

trackgbl.parameters["debug"] = 0
trackgbl.parameters["trkCollLcio"] = 'GBLTracks'
trackgbl.parameters["trkCollRoot"] = 'GBLTracks'
trackgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
trackgbl.parameters["trkRelCollLcio"] = 'TrackDataRelations'
trackgbl.parameters["trkhitCollRoot"] = 'RotatedHelicalOnTrackHits'
trackgbl.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'

#Only for detail studies
#LT uncomment
trackgbl.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack'

#LT uncommented
#if (not options.isData):
#    trackgbl.parameters["truthTrackCollLcio"] = 'GBLTracksToTruthTrackRelations'
#    trackgbl.parameters["truthTrackCollRoot"] = 'Truth_GBLTracks'

#LT check if we need the b field or not -- version of HPS java
trackgbl.parameters["bfield"] = bfield[str(options.year)]

#ECalData
ecal_gbl.parameters["debug"] = 0
ecal_gbl.parameters["hitCollLcio"] = 'EcalCalHits'
ecal_gbl.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal_gbl.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal_gbl.parameters["clusCollRoot"] = "RecoEcalClusters"

ecal_kf.parameters["debug"] = 0
ecal_kf.parameters["hitCollLcio"] = 'EcalCalHits'
ecal_kf.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal_kf.parameters["clusCollRoot"] = "RecoEcalClusters"
ecal_kf.parameters["clusCollLcio"] = "EcalClustersCorr"

#Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["vtxCollRoot"] = 'UnconstrainedV0Vertices_KF'
vtx.parameters["partCollRoot"] = 'ParticlesOnUVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

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

#MCParticle
mcpart.parameters["debug"] = 0
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

if (options.tracking == "KF"):
    sequence = [header, vtx, ecal_kf, track]
    #Get KF svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthits)
elif (options.tracking == "GBL"):
    sequence = [header, vtxgbl, ecal_gbl, trackgbl]
    #Get GBL svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthitsgbl)
elif (options.tracking == "BOTH"):
    sequence = [header, vtxgbl, ecal_gbl, trackgbl, vtx, ecal_kf, track]
    #Get KF and GBL svt truth hits
    if (options.truthHits > 0):
        sequence.append(svthits)
        sequence.append(svthitsgbl)
else:
    print("ERROR::Need to specify which tracks KF, GBL, or BOTH")

#Keep svt raw hits
if (options.rawHits > 0):
    sequence.append(rawsvt)
#If MC, get MCParticles
if (not options.isData):
    sequence.append(mcpart)

p.sequence = sequence

if (options.nevents > -1):
    p.max_events = options.nevents

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

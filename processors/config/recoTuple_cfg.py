import HpstrConf
import sys

import baseConfig as base
from baseConfig import bfield

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
track   = HpstrConf.Processor('track', 'TrackingProcessor')
trackgbl = HpstrConf.Processor('trackgbl', 'TrackingProcessor')
trackrefitgbl = HpstrConf.Processor('trackrefitgbl', 'TrackingProcessor')
svthits = HpstrConf.Processor('svthits', 'Tracker3DHitProcessor')

svthits2 = HpstrConf.Processor('svthitskf', 'Tracker2DHitProcessor')

rawsvt  = HpstrConf.Processor('rawsvt', 'SvtRawDataProcessor')
ecal    = HpstrConf.Processor('ecal', 'ECalDataProcessor')
vtx     = HpstrConf.Processor('vtx', 'VertexProcessor')
cvtx   = HpstrConf.Processor('cvtx', 'VertexProcessor')
vtxgbl   = HpstrConf.Processor('vtxgbl', 'VertexProcessor')
cvtxgbl   = HpstrConf.Processor('cvtxgbl', 'VertexProcessor')
mcpart  = HpstrConf.Processor('mcpart', 'MCParticleProcessor')
fsp = HpstrConf.Processor("fps",'FinalStateParticleProcessor')

###############################
#   Processor Configuration   #
###############################
#Event
header.parameters["debug"] = 1
header.parameters["headCollRoot"] = "EventHeader"
header.parameters["trigCollLcio"] = "TriggerBank"
header.parameters["rfCollLcio"]   = "RFHits"
header.parameters["vtpCollLcio"]  = "VTPBank"
header.parameters["vtpCollRoot"]  = "VTPBank"
header.parameters["tsCollLcio"]   = "TSBank"
header.parameters["tsCollRoot"]   = "TSBank"

#SvtRawData
rawsvt.parameters["debug"] = 1
rawsvt.parameters["hitCollLcio"]    = 'SVTRawTrackerHits'
rawsvt.parameters["hitfitCollLcio"] = 'SVTFittedRawTrackerHits'
rawsvt.parameters["hitCollRoot"]    = 'SVTRawTrackerHits'

#Tracker3DHits
svthits.parameters["debug"] = 1
svthits.parameters["hitCollLcio"]    = 'RotatedHelicalTrackHits'
svthits.parameters["hitCollRoot"]    = 'RotatedHelicalTrackHits'

#Tracker2DHits

svthits2.parameters["debug"] = 1
svthits2.parameters["hitCollLcio"]    = 'StripClusterer_SiTrackerHitStrip1D'
svthits2.parameters["hitCollRoot"]    = 'SiClusters'
svthits2.parameters["mcPartRelLcio"]    = 'SVTTrueHitRelations'

#Tracking
track.parameters["debug"] = 1
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
track.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack_KF'

track.parameters["bfield"] = bfield[str(options.year)]

trackgbl.parameters["debug"] = 1
trackgbl.parameters["trkCollLcio"] = 'GBLTracks'
trackgbl.parameters["trkCollRoot"] = 'GBLTracks'
trackgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
trackgbl.parameters["trkRelCollLcio"] = 'TrackDataRelations'
trackgbl.parameters["trkhitCollRoot"] = 'RotatedHelicalOnTrackHits'
trackgbl.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
trackgbl.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack'
trackgbl.parameters["bfield"] = bfield[str(options.year)]

#if (not options.isData):
#    trackgbl.parameters["truthTrackCollLcio"] = 'GBLTracksToTruthTrackRelations'
#    trackgbl.parameters["truthTrackCollRoot"] = 'Truth_GBLTracks'

#ECalData
ecal.parameters["debug"] = 1
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#Vertex
vtx.parameters["debug"] = 1
vtx.parameters["vtxCollLcio"]    = 'UnconstrainedV0Vertices_KF'
vtx.parameters["vtxCollRoot"]    = 'UnconstrainedV0Vertices_KF'
vtx.parameters["partCollRoot"]   = 'ParticlesOnVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

cvtxgbl.parameters["debug"] = 1
cvtxgbl.parameters["vtxCollLcio"]     = 'TargetConstrainedV0Vertices_KF'
cvtxgbl.parameters["vtxCollRoot"]     = 'TargetConstrainedV0Vertices_KF'
cvtxgbl.parameters["partCollRoot"]    = 'ParticlesOnVertices_KF'
cvtxgbl.parameters["kinkRelCollLcio"] = ''
cvtxgbl.parameters["trkRelCollLcio"]  = 'KFTrackDataRelations'


vtxgbl.parameters["debug"] = 1
vtxgbl.parameters["vtxCollLcio"]     = 'UnconstrainedV0Vertices'
vtxgbl.parameters["vtxCollRoot"]     = 'UnconstrainedV0Vertices'
vtxgbl.parameters["partCollRoot"]    = 'ParticlesOnVertices'
vtxgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
vtxgbl.parameters["trkRelCollLcio"]  = 'TrackDataRelations'

cvtxgbl.parameters["debug"] = 1
cvtxgbl.parameters["vtxCollLcio"]     = 'TargetConstrainedV0Vertices'
cvtxgbl.parameters["vtxCollRoot"]     = 'TargetConstrainedV0Vertices'
cvtxgbl.parameters["partCollRoot"]    = 'ParticlesOnVertices'
cvtxgbl.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
cvtxgbl.parameters["trkRelCollLcio"]  = 'TrackDataRelations'

#MCParticle
mcpart.parameters["debug"] = 1
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

#FinalStateParticleProcessor
fsp.parameters["debug"] = 1 
fsp.parameters["fspCollLcio"] = "FinalStateParticles_KF" 
fsp.parameters["fspCollRoot"] = "FinalStateParticles_KF"
fsp.parameters["kinkRelCollLcio"] = ""
fsp.parameters["trkRelCollLcio"] = "KFTrackDataRelations"



# Sequence which the processors will run.
if options.isData == -1: print("Please specficy if this is Data or not via option -t")
#if (not options.isData):
#    p.sequence = [header, vtx, cvtx, vtxgbl, cvtxgbl, ecal, track, trackgbl, mcpart]
#else:
    #p.sequence = [header, vtx, cvtx, vtxgbl, cvtxgbl, ecal, track, trackgbl, fsp]
    #p.sequence = [header, vtx, ecal, track]
p.sequence = [header,svthits2,fsp]

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

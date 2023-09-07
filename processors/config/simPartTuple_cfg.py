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
svthits = HpstrConf.Processor('svthits', 'Tracker2DHitProcessor')
rawsvt = HpstrConf.Processor('rawsvt', 'SvtRawDataProcessor')
mcthits = HpstrConf.Processor('mcthits', 'MCTrackerHitProcessor')
mcehits = HpstrConf.Processor('mcehits', 'MCEcalHitProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
fsp = HpstrConf.Processor('fps', 'FinalStateParticleProcessor')
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

# SvtRawData
rawsvt.parameters["debug"] = 0
rawsvt.parameters["hitCollLcio"] = 'SVTRawTrackerHits'
rawsvt.parameters["hitfitCollLcio"] = 'SVTFittedRawTrackerHits'
rawsvt.parameters["hitCollRoot"] = 'SVTRawTrackerHits'

# Tracker2DHits
svthits.parameters["debug"] = 0
svthits.parameters["hitCollLcio"] = 'StripClusterer_SiTrackerHitStrip1D'
svthits.parameters["hitCollRoot"] = 'SiClusters'
svthits.parameters["mcPartRelLcio"] = 'SVTTrueHitRelations'

# Tracking
track.parameters["debug"] = 0
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'

# Only for detail studies
# LT uncomment
track.parameters["rawhitCollRoot"] = 'SVTRawHitsOnTrack_KF'

# LT uncommented
# if (not options.isData):
#    track.parameters["truthTrackCollLcio"] = 'KalmanFullTracksToTruthTrackRelations'
#    track.parameters["truthTrackCollRoot"] = 'Truth_KFTracks'

# LT check if we need the b field or not -- version of HPS java
# for Jess's files need to give it b-field

track.parameters["bfield"] = bfield[str(options.year)]

# ECalData
ecal.parameters["debug"] = 0
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#MCTrackerHits
mcthits.parameters["debug"] = 0
mcthits.parameters["hitCollLcio"] = 'TrackerHits'
mcthits.parameters["hitCollRoot"] = 'TrackerSimHits'

#MCEcalHits
mcehits.parameters["debug"] = 0
mcehits.parameters["hitCollLcio"] = 'EcalHits'
mcehits.parameters["hitCollRoot"] = 'EcalSimHits'

#FinalStateParticleProcessor
fsp.parameters["debug"] = 0
fsp.parameters["fspCollLcio"] = "FinalStateParticles_KF"
fsp.parameters["fspCollRoot"] = "FinalStateParticles_KF"
fsp.parameters["kinkRelCollLcio"] = ""
fsp.parameters["trkRelCollLcio"] = "KFTrackDataRelations"
fsp.parameters["trkhitCollRoot"] = "fspOnTrackHits"
fsp.parameters["rawhitCollRoot"] = "fspOnTrackRawHits"
fsp.parameters["hitFitsCollLcio"] = "SVTFittedRawTrackerHits"

# MCParticle
mcpart.parameters["debug"] = 0
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

sequence = [header, ecal, track, svthits, rawsvt, mcthits, mcehits, mcpart]

p.sequence = sequence

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

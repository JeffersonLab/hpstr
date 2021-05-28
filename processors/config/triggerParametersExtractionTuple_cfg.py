import HpstrConf
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename
root_file = [options.outFilename]

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()


p.run_mode = 0

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
track = HpstrConf.Processor('track', 'TrackingProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
gtp = HpstrConf.Processor('gtp', 'ECalDataProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')

###############################
#   Processor Configuration   #
###############################
#Tracking
track.parameters["debug"] = 0 
track.parameters["trkCollLcio"] = 'GBLTracks'
track.parameters["trkCollRoot"] = 'GBLTracks'
track.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
track.parameters["trkRelCollLcio"] = 'TrackDataRelations'
track.parameters["trkhitCollRoot"] = 'RotatedHelicalOnTrackHits'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
track.parameters["rawhitCollRoot"] = '' #'SVTRawHitsOnTrack'
#track.parameters["bfield"]         = bfield[str(options.year)]


#ECalData
ecal.parameters["debug"] = 0 
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#GTP Data
gtp.parameters["debug"] = 0 
gtp.parameters["hitCollLcio"] = 'EcalClustersGTPSimHits'
gtp.parameters["hitCollRoot"] = 'RecoEcalHitsGTP'
gtp.parameters["clusCollLcio"] = "EcalClustersGTP"
gtp.parameters["clusCollRoot"] = "RecoEcalClustersGTP"

#MCParticle
mcpart.parameters["debug"] = 0 
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

# Sequence which the processors will run.
if options.isData == -1: print("Please specficy if this is Data or not via option -t")

p.sequence = [track, ecal, gtp, mcpart]

p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

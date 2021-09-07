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
header = HpstrConf.Processor('header', 'EventProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
hodo = HpstrConf.Processor('hodo', 'HodoDataProcessor')
#track = HpstrConf.Processor('track', 'TrackingProcessor')

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


#ECalData
ecal.parameters["debug"] = 0 
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#ECalData
hodo.parameters["debug"] = 0 
hodo.parameters["hitCollLcio"] = 'HodoCalHits'
hodo.parameters["hitCollRoot"] = 'RecoHodoHits'
hodo.parameters["clusCollLcio"] = "HodoGenericClusters"
hodo.parameters["clusCollRoot"] = "RecoHodoClusters"

#Tracking
#track.parameters["debug"] = 0 
#track.parameters["trkCollLcio"] = 'KalmanFullTracks'
#track.parameters["trkCollRoot"] = 'KalmanFullTracks'
#track.parameters["kinkRelCollLcio"] = ''
#track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
#track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
#track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
#track.parameters["rawhitCollRoot"] = ''

# Sequence which the processors will run.
p.sequence = [header, ecal, hodo]


p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

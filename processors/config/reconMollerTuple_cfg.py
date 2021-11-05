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
track = HpstrConf.Processor('track', 'TrackingProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
vtx = HpstrConf.Processor('vtx', 'VertexProcessor')

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

#Tracking
track.parameters["debug"] = 0 
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
track.parameters["rawhitCollRoot"] = ''


#ECalData
ecal.parameters["debug"] = 0 
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"]    = 'UnconstrainedMollerVertices_KF'
vtx.parameters["vtxCollRoot"]    = 'UnconstrainedMollerVertices_KF'
vtx.parameters["partCollRoot"]   = 'ParticlesMollerOnVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

# Sequence which the processors will run.
if options.isData == -1: print("Please specficy if this is Data or not via option -t")

p.sequence = [header, track, ecal, vtx]

p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

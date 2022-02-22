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
#svthits = HpstrConf.Processor('svthits', 'Tracker3DHitProcessor')
rawsvt = HpstrConf.Processor('rawsvt', 'SvtRawDataProcessor')
hodo = HpstrConf.Processor('hodo', 'HodoDataProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
fsp = HpstrConf.Processor('fsp', 'FinalStateParticleProcessor')
vtx = HpstrConf.Processor('vtx', 'VertexProcessor')
c_vtx = HpstrConf.Processor('c_vtx', 'VertexProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')

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

#SvtRawData
rawsvt.parameters["debug"] = 0
rawsvt.parameters["hitCollLcio"]    = 'SVTRawTrackerHits'
rawsvt.parameters["hitfitCollLcio"] = 'SVTFittedRawTrackerHits'
rawsvt.parameters["hitCollRoot"]    = 'SVTRawTrackerHits'

#Tracker3DHits
#svthits.parameters["debug"] = 0
#svthits.parameters["hitCollLcio"]    = 'RotatedHelicalTrackHits'
#svthits.parameters["hitCollRoot"]    = 'RotatedHelicalTrackHits'
#svthits.parameters["mcPartRelLcio"]  = 'RotatedHelicalTrackMCRelations'

#Tracking
track.parameters["debug"] = 0 
track.parameters["trkCollLcio"] = 'KalmanFullTracks'
track.parameters["trkCollRoot"] = 'KalmanFullTracks'
track.parameters["kinkRelCollLcio"] = ''
track.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'
track.parameters["trkhitCollRoot"] = 'SiClustersOnTrack'
track.parameters["hitFitsCollLcio"] = 'SVTFittedRawTrackerHits'
track.parameters["rawhitCollRoot"] = ''


#HodoData
hodo.parameters["debug"] = 0 
hodo.parameters["hitCollLcio"] = 'HodoCalHits'
hodo.parameters["hitCollRoot"] = 'RecoHodoHits'
hodo.parameters["clusCollLcio"] = "HodoGenericClusters"
hodo.parameters["clusCollRoot"] = "RecoHodoClusters"

#ECalData
ecal.parameters["debug"] = 0 
ecal.parameters["hitCollLcio"] = 'EcalCalHits'
ecal.parameters["hitCollRoot"] = 'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#FinalStateParticles_KF
fsp.parameters["debug"] = 0
fsp.parameters["fspCollLcio"]    = 'FinalStateParticles_KF'
fsp.parameters["fspCollRoot"]    = 'FinalStateParticles_KF'
fsp.parameters["kinkRelCollLcio"] = ''
fsp.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

#Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"]    = 'UnconstrainedV0Vertices_KF'
vtx.parameters["vtxCollRoot"]    = 'UnconstrainedV0Vertices_KF'
vtx.parameters["partCollRoot"]   = 'ParticlesOnVertices_KF'
vtx.parameters["kinkRelCollLcio"] = ''
vtx.parameters["trkRelCollLcio"] = 'KFTrackDataRelations'

#Constrained Vertex
c_vtx.parameters["debug"] = 0
c_vtx.parameters["vtxCollLcio"]     = 'TargetConstrainedV0Vertices_KF'
c_vtx.parameters["vtxCollRoot"]     = 'TargetConstrainedV0Vertices_KF'
c_vtx.parameters["partCollRoot"]    = 'ParticlesOnConstrainedVertices_KF'
c_vtx.parameters["kinkRelCollLcio"] = ''
c_vtx.parameters["trkRelCollLcio"]  = 'KFTrackDataRelations'


#MCParticle
mcpart.parameters["debug"] = 0 
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

# Sequence which the processors will run.
if options.isData == -1: print("Please specficy if this is Data or not via option -t")
if options.isData == 1: 
    p.sequence = [header, track, rawsvt, hodo, ecal, fsp, vtx, c_vtx]
    #p.sequence = [header, track, ecal, fsp, vtx, c_vtx]
else: 
    #p.sequence = [header, track, ecal, fsp, vtx, c_vtx, mcpart]
    p.sequence = [header, track, rawsvt, hodo, ecal, fsp, vtx, c_vtx, mcpart]

p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

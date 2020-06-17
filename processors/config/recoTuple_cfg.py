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
svthits = HpstrConf.Processor('svthits', 'Tracker3DHitProcessor')
rawsvt = HpstrConf.Processor('rawsvt', 'SvtRawDataProcessor')
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
svthits.parameters["debug"] = 0
svthits.parameters["hitCollLcio"]    = 'RotatedHelicalTrackHits'
svthits.parameters["hitCollRoot"]    = 'RotatedHelicalTrackHits'
svthits.parameters["mcPartRelLcio"]  = 'RotatedHelicalTrackMCRelations'


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
ecal.parameters["hitCollRoot"] = ''#'RecoEcalHits'
ecal.parameters["clusCollLcio"] = "EcalClustersCorr"
ecal.parameters["clusCollRoot"] = "RecoEcalClusters"

#FinalStateParticles
fsp.parameters["debug"] = 0
fsp.parameters["fspCollLcio"]    = 'FinalStateParticles'
fsp.parameters["fspCollRoot"]    = 'FinalStateParticles'
fsp.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
fsp.parameters["trkRelCollLcio"] = 'TrackDataRelations'

#Vertex
vtx.parameters["debug"] = 0
vtx.parameters["vtxCollLcio"]    = 'UnconstrainedV0Vertices'
vtx.parameters["vtxCollRoot"]    = 'UnconstrainedV0Vertices'
vtx.parameters["partCollRoot"]   = 'ParticlesOnVertices'
vtx.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
vtx.parameters["trkRelCollLcio"] = 'TrackDataRelations'

#Constrained Vertex
c_vtx.parameters["debug"] = 0
c_vtx.parameters["vtxCollLcio"]     = 'TargetConstrainedV0Vertices'
c_vtx.parameters["vtxCollRoot"]     = 'TargetConstrainedV0Vertices'
c_vtx.parameters["partCollRoot"]    = 'ParticlesOnConstrainedVertices'
c_vtx.parameters["kinkRelCollLcio"] = 'GBLKinkDataRelations'
c_vtx.parameters["trkRelCollLcio"]  = 'TrackDataRelations'


#MCParticle
mcpart.parameters["debug"] = 0 
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

# Sequence which the processors will run.
if options.isData == -1: print("Please specficy if this is Data or not via option -t")
if options.isData == 1: 
    #p.sequence = [header, track, rawsvt, svthits, ecal, fsp, vtx, c_vtx]
    p.sequence = [header, track, ecal, fsp, vtx, c_vtx]
else: 
    p.sequence = [header, track, ecal, fsp, vtx, c_vtx, mcpart]
    #p.sequence = [header, track, rawsvt, svthits, ecal, fsp, vtx, c_vtx, mcpart]

p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

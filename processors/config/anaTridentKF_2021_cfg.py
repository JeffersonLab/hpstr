import HpstrConf
import sys
import os
import baseConfig

options = baseConfig.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print('Input file: %s' % infile)
print('Output file: %s' % outfile)

p = HpstrConf.Process()

p.run_mode = 1
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

vtxana = HpstrConf.Processor('vtxana', 'TridentWABAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
vtxana.parameters["debug"] = 0
vtxana.parameters["anaName"] = "vtxana"
vtxana.parameters["cluColl"] = "RecoEcalClusters"
vtxana.parameters["trkColl"] = "KalmanFullTracks"
vtxana.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"
#vtxana.parameters["hitColl"]  = "RotatedHelicalTrackHits"
#vtxana.parameters["rawhitColl"]  = "fspOnTrackRawHits"
#vtxana.parameters["hitColl"]  = "fspOnTrackHits"
vtxana.parameters["mcColl"]  = "MCParticle"
vtxana.parameters["fspartColl"] = "FinalStateParticles_KF"
#vtxana.parameters["fspartColl"] = "ParticlesOnVertices_KF"
vtxana.parameters["trkSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/tridents/trackSelection-3pt7gev.json'
vtxana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/tridents/tridentSelection-3pt7gev.json'
#vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis.json"
vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tridents/triAnalysis-3pt7gev.json"
vtxana.parameters["beamE"] = 3.7
vtxana.parameters["isData"] = options.isData
#vtxana.parameters["isData"] = 1
CalTimeOffset=-999

#if (options.isData==1):
#    CalTimeOffset=56.
#    print "Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset
    
#elif (options.isData==0):
#    CalTimeOffset=43.
#    print "Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset
#else:
#    print "Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]"
#below are 2019 numbers
if (options.isData==1):
    CalTimeOffset=38.
    print("Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset)
    TrkTimeOffset=-17.
    print("Running on data file: Setting TrkTimeOffset %d"  % TrkTimeOffset)

elif (options.isData==0):
    CalTimeOffset=27.
    print ("Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset)
    TrkTimeOffset=-30.
    print ("Running on MC file: Setting TrkTimeOffset %d"  % TrkTimeOffset)

vtxana.parameters["CalTimeOffset"]=CalTimeOffset
vtxana.parameters["TrkTimeOffset"]=TrkTimeOffset

#Region definitions

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/tridents/"
#vtxana.parameters["regionDefinitions"] = [RegionPath+'Tight.json']
#vtxana.parameters["regionDefinitions"] = ['./tridentL1L1.json','./tridentL1L2.json','./tridentL2L1.json','./tridentL2L2.json']
#vtxana.parameters["regionDefinitions"] = ['cfgs2019/tridentAllLayerCombos.json']
vtxana.parameters["regionDefinitions"] = [RegionPath+'/tridentAllLayerCombos.json',
                                          RegionPath+'/tridentAllLayerCombos-NoClusters.json', 
                                          RegionPath+'/tridentAllLayerCombos-NoPosClust-EleClust.json', 
                                          RegionPath+'/tridentAllLayerCombos-PosClust-NoEleClust.json',  
                                          RegionPath+'/tridentAllLayerCombos-BothClusters.json',
                                          RegionPath+'/tridentL2Required.json',
                                          RegionPath+'/tridentL2Required-NoClusters.json',
                                          RegionPath+'/tridentL2Required-BothClusters.json',
                                          RegionPath+'/tridentL2Required-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentL2Required-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentL1L1.json',
                                          RegionPath+'/tridentL1L1-NoClusters.json',
                                          RegionPath+'/tridentL1L1-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentL1L1-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentL1L1-BothClusters.json',
                                          RegionPath+'/tridentL2L1.json',
                                          RegionPath+'/tridentL2L1-NoClusters.json',
                                          RegionPath+'/tridentL2L1-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentL2L1-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentL2L1-BothClusters.json',
                                          RegionPath+'/tridentL1L2.json',
                                          RegionPath+'/tridentL1L2-NoClusters.json',
                                          RegionPath+'/tridentL1L2-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentL1L2-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentL1L2-BothClusters.json',
                                          RegionPath+'/tridentL2L2.json',
                                          RegionPath+'/tridentL2L2-NoClusters.json',
                                          RegionPath+'/tridentL2L2-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentL2L2-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentL2L2-BothClusters.json',
                                          RegionPath+'/tridentNoInnerMatch-NoClusters.json',
                                          RegionPath+'/tridentNoInnerMatch-NoPosClust-EleClust.json',
                                          RegionPath+'/tridentNoInnerMatch-PosClust-NoEleClust.json',
                                          RegionPath+'/tridentNoInnerMatch-BothClusters.json']                                  
#vtxana.parameters["regionWABDefinitions"]=[RegionPath+'/wabL1ElectronCluster.json']
# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files=infile
p.output_files = [outfile]

p.printProcess()



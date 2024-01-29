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
vtxana.parameters["debug"] = 1
vtxana.parameters["anaName"] = "vtxana"
vtxana.parameters["cluColl"] = "RecoEcalClusters"
vtxana.parameters["trkColl"] = "KalmanFullTracks"
vtxana.parameters["vtxColl"] = "UnconstrainedV0Candidates_KF"
#vtxana.parameters["hitColl"]  = "RotatedHelicalTrackHits"
vtxana.parameters["hitColl"]  = "SiClustersOnTrack"
vtxana.parameters["mcColl"]  = "MCParticle"
#vtxana.parameters["fspartColl"] = "FinalStateParticles_KF"
vtxana.parameters["fspartColl"] = "ParticlesOnVertices_KF"
vtxana.parameters["trkSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/tridents/trackSelection-4pt5gev.json'
vtxana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/tridents/tridentSelection-4pt5gev.json'
#vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis.json"
vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tridents/triAnalysis-4pt5gev.json"
vtxana.parameters["beamE"] = 4.5
vtxana.parameters["isData"] = options.isData
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
    CalTimeOffset=43.
    print("Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset)
    
elif (options.isData==0):
    CalTimeOffset=27.
    print ("Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset)

vtxana.parameters["CalTimeOffset"]=CalTimeOffset

#Region definitions

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/tridents/"
#vtxana.parameters["regionDefinitions"] = [RegionPath+'Tight.json']
#vtxana.parameters["regionDefinitions"] = ['./tridentL1L1.json','./tridentL1L2.json','./tridentL2L1.json','./tridentL2L2.json']
#vtxana.parameters["regionDefinitions"] = ['cfgs2019/tridentAllLayerCombos.json']
vtxana.parameters["regionDefinitions"] = [RegionPath+'/tridentAllLayerCombos.json',
                                          RegionPath+'/tridentAllLayerCombos-PositronCluster.json', 
                                          RegionPath+'/tridentAllLayerCombos-PosClust-NoEleClust.json',  
                                          RegionPath+'/tridentAllLayerCombos-BothClusters.json',
                                          RegionPath+'/tridentAllLayerCombos-RadCut.json', 
                                          RegionPath+'/tridentAllLayerCombos-PositronCluster-RadCut.json', 
                                          RegionPath+'/tridentAllLayerCombos-PosClust-NoEleClust-RadCut.json',  
                                          RegionPath+'/tridentAllLayerCombos-BothClusters-RadCut.json', 
                                          RegionPath+'/tridentAllLayerCombos-GammaStTruth-RadCut.json', 
                                          RegionPath+'/tridentAllLayerCombos-GammaStTruth-PositronCluster-RadCut.json', 
                                          RegionPath+'/tridentAllLayerCombos-GammaStTruth-BothClusters-RadCut.json', 
                                          RegionPath+'/tridentL1L1.json',
                                          RegionPath+'/tridentL1L1-PositronCluster.json',
                                          RegionPath+'/tridentL1L1-BothClusters.json',
                                          RegionPath+'/tridentL1L1-PositronCluster-RadCut.json',
                                          RegionPath+'/tridentL1L1-BothClusters-RadCut.json']
#                                          RegionPath+'/tridentL2L1.json',
#                                          RegionPath+'/tridentL2L1-PositronCluster.json',
#                                          RegionPath+'/tridentL2L1-BothClusters.json',
#                                          RegionPath+'/tridentL1L2.json',
#                                          RegionPath+'/tridentL1L2-PositronCluster.json',
#                                          RegionPath+'/tridentL1L2-BothClusters.json',
#                                          RegionPath+'/tridentL2L2.json',
#                                          RegionPath+'/tridentL2L2-PositronCluster.json',
#                                          RegionPath+'/tridentL2L2-BothClusters.json']
vtxana.parameters["regionWABDefinitions"]=[RegionPath+'/wabL1ElectronCluster.json']
# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files=infile
p.output_files = [outfile]

p.printProcess()



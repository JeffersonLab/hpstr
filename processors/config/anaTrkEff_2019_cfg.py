import HpstrConf
import sys
import os
import baseConfig

#(options,args) = baseConfig.parser.parse_args()
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

vtxana = HpstrConf.Processor('vtxana', 'TrackEfficiencyProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
vtxana.parameters["debug"] = 1
vtxana.parameters["anaName"] = "vtxana"
#vtxana.parameters["trkColl"] = "GBLTracks"
vtxana.parameters["trkColl"] = "KalmanFullTracks"
vtxana.parameters["cluColl"] = "RecoEcalClusters"
#vtxana.parameters["fspartColl"] = "FinalStateParticles"
vtxana.parameters["fspartColl"] = "FinalStateParticles_KF"
vtxana.parameters["cluSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackEfficiency/2019/clusterSelection-2019.json'
vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/trackEfficiency/trkEffPlotConfig-4.5gev.json"
vtxana.parameters["thrProngCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/trackEfficiency/threeProngPlotConfig-4.5gev.json"
vtxana.parameters["cluHistoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/trackEfficiency/trkEffPreSelPlotConfig-4.5gev.json"
vtxana.parameters["beamE"] = 4.5
#vtxana.parameters["isData"] = options.isData
vtxana.parameters["isData"] = 1
CalTimeOffset=-999

if (options.isData==1):
    CalTimeOffset=37.
    print("Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset)
    
elif (options.isData==0):
    CalTimeOffset=27.
    print("Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


vtxana.parameters["CalTimeOffset"]=CalTimeOffset

#Region definitions

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/trackEfficiency/2019/"
vtxana.parameters["regionDefinitions"] = [RegionPath+"trkEff2InTrigClusters.json",
                                          RegionPath+"trkEffFiducial2InTrigClusters.json",
                                          RegionPath+"trkEffTriCut2InTrigClusters.json",   
                                          RegionPath+"trkEffFiducialTriCut2InTrigClusters.json", 
                                          RegionPath+"trkEffWABCut2InTrigClusters.json", 
                                          RegionPath+"trkEffFiducialWABCut2InTrigClusters.json"]

vtxana.parameters["threeProngDefinitions"]=[RegionPath+"trkEff3ProngCut3InTrigClusters.json"]

# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files=infile
p.output_files = [outfile]

p.printProcess()



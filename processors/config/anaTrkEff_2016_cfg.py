import HpstrConf
import sys
import os
import baseConfig

(options,args) = baseConfig.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

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
vtxana.parameters["trkColl"] = "GBLTracks"
vtxana.parameters["cluColl"] = "RecoEcalClusters"
vtxana.parameters["fspartColl"] = "FinalStateParticles"
vtxana.parameters["cluSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/clusterSelection-2016.json'
vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/trkeff/trkEffPlotConfig-2.3gev.json"
vtxana.parameters["cluHistoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/trkeff/trkEffPreSelPlotConfig-2.3gev.json"
vtxana.parameters["beamE"] = 2.3
vtxana.parameters["isData"] = options.isData
CalTimeOffset=-999

if (options.isData==1):
    CalTimeOffset=56.
    print "Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset
    
elif (options.isData==0):
    CalTimeOffset=43.
    print "Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset
else:
    print "Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]"


vtxana.parameters["CalTimeOffset"]=CalTimeOffset

#Region definitions

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/"
vtxana.parameters["regionDefinitions"] = [RegionPath+'trkEffFiducial.json',RegionPath+'trkEffAllECal.json']

# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files=[infile]
p.output_files = [outfile]

p.printProcess()



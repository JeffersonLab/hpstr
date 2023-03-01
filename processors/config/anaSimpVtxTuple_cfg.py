import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument("-f", "--makeFlatTuple", type=int, dest="makeFlatTuple", help="Make True to make vertex ana flat tuple", metavar="makeFlatTuple", default=0)

options = base.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print('Input file: %s' % infile)
print('Output file: %s' % outfile)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

vtxana = HpstrConf.Processor('vtxana', 'VertexAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
vtxana.parameters["debug"] = 0
vtxana.parameters["anaName"] = "vtxana"
vtxana.parameters["trkColl"] = "GBLTracks"
vtxana.parameters["hitColl"] = "RotatedHelicalOnTrackHits"
vtxana.parameters["vtxColl"] = "UnconstrainedV0Vertices"
vtxana.parameters["mcColl"] = "MCParticle"
vtxana.parameters["ecalColl"] = "RecoEcalClusters"
vtxana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/empty.json'
vtxana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis.json"
vtxana.parameters["beamE"] = base.beamE[str(options.year)]
vtxana.parameters["isData"] = options.isData
vtxana.parameters["makeFlatTuple"] = options.makeFlatTuple

CalTimeOffset = -999

if (options.isData == 1):
    CalTimeOffset = 56.
    print("Running on data file: Setting CalTimeOffset %d" % CalTimeOffset)

elif (options.isData == 0):
    CalTimeOffset = 43.
    print("Running on MC file: Setting CalTimeOffset %d" % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


vtxana.parameters["CalTimeOffset"] = CalTimeOffset

#Region definitions

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/"

vtxana.parameters["regionDefinitions"] = [RegionPath+'vertexSelection.json',
                                          RegionPath+'simpTight.json',
                                          RegionPath+'simpTightL1L1.json',
                                          RegionPath+'simpTightL1L1NoSharedL0.json',
                                          RegionPath+'simpTightVtxY.json']

# Sequence which the processors will run.
p.sequence = [vtxana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()

import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument("-f", "--makeFlatTuple", type=int, dest="makeFlatTuple", help="Make True to make vertex ana flat tuple", metavar="makeFlatTuple", default=0)

options = base.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print('Input file: {}'.format(infile))
print('Output file: {}'.format(outfile))

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

bhana = HpstrConf.Processor('bhana', 'VertexAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
bhana.parameters["debug"] = 1
bhana.parameters["anaName"] = "bhana"
bhana.parameters["trkColl"] = "GBLTracks"
bhana.parameters["hitColl"] = "RotatedHelicalTrackHits"
bhana.parameters["vtxColl"] = "TargetConstrainedV0Vertices"
bhana.parameters["mcColl"] = "MCParticle"
bhana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/bhSelection.json'
bhana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis.json"
bhana.parameters["beamE"] = 2.3
bhana.parameters["isData"] = options.isData
bhana.parameters["makeFlatTuple"] = options.makeFlatTuple
CalTimeOffset = -999

if (options.isData == 1):
    CalTimeOffset = 56.
    print("Running on data file: Setting CalTimeOffset %d" % CalTimeOffset)

elif (options.isData == 0):
    CalTimeOffset = 43.
    print("Running on MC file: Setting CalTimeOffset %d" % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


bhana.parameters["CalTimeOffset"] = CalTimeOffset

#Region definitions

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/"
bhana.parameters["regionDefinitions"] = [RegionPath+'bhTight.json',
                                         RegionPath+'bhRadFracRad.json',
                                         RegionPath+'bhRadFracRadRafo.json',
                                         RegionPath+'bhRadFracRecoil.json'
                                         ]

# Sequence which the processors will run.
p.sequence = [bhana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()

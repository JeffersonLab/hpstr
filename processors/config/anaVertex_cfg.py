import HpstrConf
import sys
import os
import baseConfig as base


options = base.parser.parse_args()


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

vtxAna = HpstrConf.Processor('vertexAna', 'VtxAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
vtxAna.parameters["debug"] = 0
vtxAna.parameters["anaName"] = "vtxAna"
vtxAna.parameters["TSColl"] = "TSBank"
vtxAna.parameters["trkColl"] = "KalmanFullTracks"
vtxAna.parameters["vtxColl"] = "UnconstrainedV0Vertices_KF"
vtxAna.parameters["tcvtxColl"] = "TargetConstrainedV0Vertices_KF"
vtxAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/vertex/vertex.json'
vtxAna.parameters["beamE"] = base.beamE[str(options.year)]

vtxAna.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/2019/vertexSelection_noChi2Cut.json'

vtxAna.parameters["beamE"] = base.beamE[str(options.year)]
vtxAna.parameters["isData"] = options.isData

CalTimeOffset=-999
if (options.isData==1):
    CalTimeOffset=39.
    print("Running on data file: Setting CalTimeOffset %d"  % CalTimeOffset)
    
elif (options.isData==0):
    CalTimeOffset=39.
    print("Running on MC file: Setting CalTimeOffset %d"  % CalTimeOffset)
else:
    print("Specify which type of ntuple you are running on: -t 1 [for Data] / -t 0 [for MC]")


vtxAna.parameters["CalTimeOffset"]=CalTimeOffset


# Sequence which the processors will run.
p.sequence = [vtxAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

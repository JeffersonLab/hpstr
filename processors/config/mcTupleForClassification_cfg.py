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

mcTupleForClassification = HpstrConf.Processor('mcTupleForClassification', 'MCTupleForClassificationProcessor')

###############################
#   Processor Configuration   #
###############################
#Vertex Analysis
mcTupleForClassification.parameters["debug"] = 1
mcTupleForClassification.parameters["anaName"] = "mcTuple"
mcTupleForClassification.parameters["trkColl"] = "GBLTracks"
mcTupleForClassification.parameters["hitColl"] = "RotatedHelicalOnTrackHits"
mcTupleForClassification.parameters["vtxColl"] = "UnconstrainedV0Vertices"
mcTupleForClassification.parameters["mcColl"]  = "MCParticle"
mcTupleForClassification.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/MCTupleForClassification2019.json'
#mcTupleForClassification.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/vtxAnalysis2019.json"
mcTupleForClassification.parameters["beamE"] = base.beamE[str(options.year)]
mcTupleForClassification.parameters["isData"] = options.isData

# Sequence which the processors will run.
p.sequence = [mcTupleForClassification]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()



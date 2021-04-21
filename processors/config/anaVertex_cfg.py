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
vtxAna.parameters["vtxColl"] = "UnconstrainedV0Vertices"
vtxAna.parameters["tcvtxColl"] = "TargetConstrainedV0Vertices"
vtxAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/vertex/vertex.json'
vtxAna.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [vtxAna]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()

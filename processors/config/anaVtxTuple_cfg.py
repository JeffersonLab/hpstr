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
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

recoana = HpstrConf.Processor('vtxana', 'VertexAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
recoana.parameters["debug"] = 1
recoana.parameters["anaName"] = "vtxana"
recoana.parameters["trkColl"] = "GBLTracks"
recoana.parameters["vtxColl"] = "UnconstrainedV0Vertices"
recoana.parameters["vtxSelectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/vertexSelection.json'
recoana.parameters["histoCfg"] = os.environ['HPSTR_BASE']+"/analysis/plotconfigs/tracking/basicTracking.json"

# Sequence which the processors will run.
p.sequence = [recoana]

p.input_files=[infile]
p.output_files = [outfile]

p.printProcess()



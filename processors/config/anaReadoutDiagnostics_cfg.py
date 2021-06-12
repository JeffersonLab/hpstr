import HpstrConf
import sys
import os
import baseConfig as base

#options = base.parser.parse_args()

options = base.parser.parse_args()


print(options)

# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

analysis = options.analysis

print('Input file: %s' % infile)
print('Output file: %s' % outfile)
print('Analysis : %s' % analysis)

p = HpstrConf.Process()

p.run_mode = 1

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

readoutDiagnosticsAnaProcessor = HpstrConf.Processor('readoutDiagnosticsAnaProcessor', 'ReadoutDiagnosticsAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
readoutDiagnosticsAnaProcessor.parameters["debug"] = 0
readoutDiagnosticsAnaProcessor.parameters["anaName"] = "readoutDiagnosticsAnaProcessor"
readoutDiagnosticsAnaProcessor.parameters["ecalClusColl"] = "RecoEcalClusters"
readoutDiagnosticsAnaProcessor.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/readoutDiagnostics/readoutDiagnostics.json'
readoutDiagnosticsAnaProcessor.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [readoutDiagnosticsAnaProcessor]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

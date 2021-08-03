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

triggerParametersExtractionFEEAna = HpstrConf.Processor('triggerParametersExtractionFEEAna', 'TriggerParametersExtractionFEEAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerParametersExtractionFEEAna.parameters["debug"] = 0
triggerParametersExtractionFEEAna.parameters["anaName"] = "triggerParameterExtractionAna"
triggerParametersExtractionFEEAna.parameters["trkColl"] = "GBLTracks"
triggerParametersExtractionFEEAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
triggerParametersExtractionFEEAna.parameters["analysis"] = analysis
triggerParametersExtractionFEEAna.parameters["beamE"] = base.beamE[str(options.year)]
if (options.year == 20211920) : 
    triggerParametersExtractionFEEAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionFEE20211920.json'
else:
    triggerParametersExtractionFEEAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionFEE.json' 

# Sequence which the processors will run.
p.sequence = [triggerParametersExtractionFEEAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

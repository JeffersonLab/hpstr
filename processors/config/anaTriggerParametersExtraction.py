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

triggerParametersExtractionAna = HpstrConf.Processor('triggerParametersExtractionAna', 'TriggerParametersExtractionAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerParametersExtractionAna.parameters["debug"] = 0
triggerParametersExtractionAna.parameters["anaName"] = "triggerParameterExtractionAna"
triggerParametersExtractionAna.parameters["trkColl"] = "GBLTracks"
triggerParametersExtractionAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
triggerParametersExtractionAna.parameters["analysis"] = analysis
triggerParametersExtractionAna.parameters["beamE"] = base.beamE[str(options.year)]
if (options.year == 20211920) : 
    triggerParametersExtractionAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtraction20211920.json'
else:
    triggerParametersExtractionAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtraction.json' 

# Sequence which the processors will run.
p.sequence = [triggerParametersExtractionAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

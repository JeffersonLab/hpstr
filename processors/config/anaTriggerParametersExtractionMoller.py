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

triggerParametersExtractionMollerAna = HpstrConf.Processor('triggerParametersExtractionMollerAna', 'TriggerParametersExtractionMollerAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerParametersExtractionMollerAna.parameters["debug"] = 0
triggerParametersExtractionMollerAna.parameters["anaName"] = "triggerParameterExtractionAna"
triggerParametersExtractionMollerAna.parameters["trkColl"] = "KalmanFullTracks"
triggerParametersExtractionMollerAna.parameters["gtpClusColl"] = "RecoEcalClustersGTP"
triggerParametersExtractionMollerAna.parameters["vtxColl"] = "UnconstrainedMollerVertices"
triggerParametersExtractionMollerAna.parameters["analysis"] = analysis
triggerParametersExtractionMollerAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerParametersExtraction/triggerParametersExtractionMoller.json'

# Sequence which the processors will run.
p.sequence = [triggerParametersExtractionMollerAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

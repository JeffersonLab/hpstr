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

triggerValidationAna = HpstrConf.Processor('triggerValidationAna', 'TriggerValidationAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
triggerValidationAna.parameters["debug"] = 0
triggerValidationAna.parameters["anaName"] = "triggerValidationAna"
triggerValidationAna.parameters["VTPColl"] = "VTPBank"
triggerValidationAna.parameters["TSColl"] = "TSBank"
triggerValidationAna.parameters["ecalClusColl"] = "RecoEcalClusters"
triggerValidationAna.parameters["hodoHitColl"] = "RecoHodoHits"
triggerValidationAna.parameters["hodoClusColl"] = "RecoHodoClusters"
triggerValidationAna.parameters["trkColl"] = "KalmanFullTracks"
triggerValidationAna.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/triggerValidation/triggerValidation.json'
triggerValidationAna.parameters["beamE"] = base.beamE[str(options.year)]

# Sequence which the processors will run.
p.sequence = [triggerValidationAna]

p.input_files = infile
p.output_files = [outfile]

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

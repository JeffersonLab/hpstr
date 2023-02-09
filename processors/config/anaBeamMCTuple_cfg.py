import HpstrConf
import sys
import os
import baseConfig as base

#options = base.parser.parse_args()


base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
                  help="Which tracking to use to make plots", metavar="tracking", default="KF")
options = base.parser.parse_args()


print(options)

# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

analysis = "beam"



print('Input file: %s' % infile)
print('Output file: %s' % outfile)
print('Analysis : %s' % analysis)

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

mcana = HpstrConf.Processor('mcana', 'MCAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
mcana.parameters["debug"] = 0
mcana.parameters["anaName"] = "mcAna"
mcana.parameters["partColl"] = "MCParticle"
mcana.parameters["trkrHitColl"] = "TrackerHits"
mcana.parameters["ecalHitColl"] = "EcalHits"
mcana.parameters["analysis"] = analysis
mcana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/beamMC.json'

# Sequence which the processors will run.
p.sequence = [mcana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()

import HpstrConf
import sys

import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename
root_file = options.outFilename

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()

p.run_mode = 0
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')
mcthits = HpstrConf.Processor('mcthits', 'MCTrackerHitProcessor')
mcehits = HpstrConf.Processor('mcehits', 'MCEcalHitProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
mcpart.parameters["debug"] = 0
mcpart.parameters["mcPartCollLcio"] = 'MCParticle'
mcpart.parameters["mcPartCollRoot"] = 'MCParticle'

#MCTrackerHits
mcthits.parameters["debug"] = 0
mcthits.parameters["hitCollLcio"] = 'TrackerHits'
mcthits.parameters["hitCollRoot"] = 'TrackerHits'

#MCEcalHits
mcehits.parameters["debug"] = 0
mcehits.parameters["hitCollLcio"] = 'EcalHits'
mcehits.parameters["hitCollRoot"] = 'EcalHits'

# Sequence which the processors will run.
#p.sequence = [mcpart, mcthits, mcehits]
p.sequence = [mcpart]

p.input_files = lcio_file
p.output_files = [root_file]

p.printProcess()

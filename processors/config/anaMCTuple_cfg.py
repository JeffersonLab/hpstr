import HpstrConf
import sys
import os

# Use the input file to set the output file name
infile = sys.argv[1].strip()
outfile = '%s_anaMC.root' % infile[:-5]

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

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
mcana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/basicMC.json'

# Sequence which the processors will run.
p.sequence = [mcana]

p.input_files=[infile]
p.output_files = [outfile]

#p.max_events = 1000

p.printProcess()

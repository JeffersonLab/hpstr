import HpstrConf
import sys

# Use the input file to set the output file name
lcio_file = sys.argv[1].strip()
root_file = '%s.root' % lcio_file[:-6]

print 'LCIO file: %s' % lcio_file
print 'Root file: %s' % root_file

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

header = HpstrConf.Processor('header', 'EventProcessor')
track = HpstrConf.Processor('svt', 'TrackingProcessor')
#track = HpstrConf.Processor('svt', 'SvtDataProcessor')
ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')
mcpart = HpstrConf.Processor('mcpart', 'MCParticleProcessor')

# Sequence which the processors will run.
p.sequence = [header, track, ecal, mcpart]

p.input_files=[lcio_file]
p.output_files = [root_file]

#p.max_events = 1000

p.printProcess()

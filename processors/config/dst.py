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
svt = HpstrConf.Processor('svt', 'SvtDataProcessor')

ecal = HpstrConf.Processor('ecal', 'ECalDataProcessor')

particle = HpstrConf.Processor("particle", "ParticleProcessor")
particle.parameters["Collections"] = [ 'FinalStateParticles', 
                                       'BeamspotConstrainedMollerCandidates', 
                                       'BeamspotConstrainedV0Candidates', 
                                       'TargetConstrainedMollerCandidates', 
                                       'TargetConstrainedV0Candidates',
                                       'UnconstrainedMollerCandidates', 
                                       'UnconstrainedV0Candidates',
                                       'UnconstrainedVcCandidates',
                                       'OtherElectrons'
                                     ]

# Sequence which the processors will run.
p.sequence = [header, svt, ecal, particle]

p.input_files=[lcio_file]
p.output_files = [root_file]

#p.max_events = 1000

p.printProcess()

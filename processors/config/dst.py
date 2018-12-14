#
# Configuration used to generate DST's
#

import HpstrConf

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

header   = HpstrConf.Processor('header', 'EventProcessor')

svt      = HpstrConf.Processor('svt', 'SvtDataProcessor')

ecal     = HpstrConf.Processor('ecal', 'ECalDataProcessor')

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

p.input_files=['input.lcio']
p.output_files = ['dst.root']

#p.max_events = 100

p.printProcess()

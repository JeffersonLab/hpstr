import HpstrConf
import baseConfig as base
import os


options = base.parser.parse_args()

# Use the input file to set the output file name
stdhep_file = options.inFilename[0]
root_file = options.outFilename


p = HpstrConf.Process()

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

cnvStd = HpstrConf.Processor('cnvStd', 'StdhepMCParticleProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
cnvStd.parameters["mcPartCollStdhep"] = 'MCParticle'
cnvStd.parameters["mcPartCollRoot"] = 'MCParticle'
cnvStd.parameters["skipEvent"] = options.skip_events
if options.nevents > -1:
    cnvStd.parameters["maxEvent"] = options.skip_events+options.nevents
else:
    cnvStd.parameters["maxEvent"] = -1
# Sequence which the processors will run.
p.sequence = [cnvStd]

p.input_files = [stdhep_file]
#p.input_files=[]
p.output_files = [root_file]

p.printProcess()

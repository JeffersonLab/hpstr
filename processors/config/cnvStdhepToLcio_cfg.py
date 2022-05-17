import HpstrConf
import baseConfig as base
import os


options = base.parser.parse_args()

# Use the input file to set the output file name
stdhep_file = options.inFilename[0]
root_file = options.outFilename


p = HpstrConf.Process()

p.run_mode = 2

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

cnvStd = HpstrConf.Processor('cnvStd', 'StdHepToLcioProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
cnvStd.parameters["mcPartCollStdhep"] = 'MCParticle'
cnvStd.parameters["mcPartCollRoot"] = 'MCParticle'
cnvStd.parameters["maxEvent"] = options.nevents

# Sequence which the processors will run.
p.sequence = [cnvStd]

p.input_files=[stdhep_file]
#p.input_files=[]
p.output_files = [root_file]

p.printProcess()

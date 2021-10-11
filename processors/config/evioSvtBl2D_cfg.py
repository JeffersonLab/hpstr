import HpstrConf
import sys
import os
import baseConfig

baseConfig.parser.add_argument("-c", "--chNumCfg", type=str, dest="chNumCfg", action='store',
                  help="Configuration for channel numbering.", metavar="chNumCfg", default="fw")
baseConfig.parser.add_argument("-N", "--histNames", type=str, dest="histNames", action='store',
                  help="Configuration for histogram naming convention.", 
                  metavar="histNames", default="fw")

options = baseConfig.parser.parse_args()

# Use the input file to set the output file name
in_file  = options.inFilename[0]
out_file = options.outFilename

print('In file: %s' % in_file)
print('Out file: %s' % out_file)

p = HpstrConf.Process()

p.run_mode = 3
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

evio = HpstrConf.Processor('evio', 'SvtBl2DEvioProcessor')

###############################
#   Processor Configuration   #
###############################

#evio
evio.parameters["debug"]    = 0
evio.parameters["trigConf"] = "hps_v12_1.cnf"
evio.parameters["chNumCfg"] = options.chNumCfg
evio.parameters["histNames"] = options.histNames
evio.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/Svt2DBlHw.json'

# Sequence which the processors will run.
p.sequence = [evio]

p.input_files=[in_file]
p.output_files = [out_file]

p.printProcess()

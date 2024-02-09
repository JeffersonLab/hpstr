import HpstrConf
import sys
import baseConfig as base
from baseConfig import bfield

base.parser.add_argument("-L", "--layer", type=int, dest="layer",
                  help="Layer Under Investigation", metavar="layer",default=-1)
base.parser.add_argument("-M", "--module", type=int, dest="module",
                  help="Module Under Investigation", metavar="module",default=-1)
base.parser.add_argument("-MC", "--MC", type=int, dest="isMC",
                  help="IS IT MC", metavar="module",default=0)
base.parser.add_argument("-doT", "--doT", type=int, dest="doTrack",
                  help="DO WE DO TRACK STUFF", metavar="doTrack",default=0)
base.parser.add_argument("-cut", "--cut", type=float, dest="cut",
                  help="Momentum Cut for NShared Profile", metavar="cut",default=-1.0)


options = base.parser.parse_args()

# Use the input file to set the output file name
root_file = options.inFilename
ana_file = options.outFilename

#print('LCIO file: %s' % root_file)
#print('Root file: %s' % ana_file)

p = HpstrConf.Process()

#p.max_events = 1000
p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
rclu = HpstrConf.Processor('rclu','RoryClusterAnaProcessor')
rclu.parameters["debug"] = 0
rclu.parameters["layer"] = options.layer
rclu.parameters["module"] = options.module
rclu.parameters["isMC"] = options.isMC
rclu.parameters["doTrack"] = options.doTrack
rclu.parameters["cut"] = options.cut

sequence = [rclu]        

p.sequence = sequence

print("THE INPUT FILE IS "+str(root_file))
p.input_files = root_file
p.output_files = [ana_file]

p.printProcess()

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
base.parser.add_argument("-fn1", "--fn1", type=str, dest="fileName1",
                  help="Name for the First File", metavar="fileName1",default="")
base.parser.add_argument("-fn2", "--fn2", type=str, dest="fileName2",
                  help="Name for the Second File", metavar="fileName2",default="")




options = base.parser.parse_args()

# Use the input file to set the output file name
root_files = [options.fileName1]#,options.fileName2]
ana_file = options.outFilename

#print('LCIO file: %s' % root_file)
#print('Root file: %s' % ana_file)

p = HpstrConf.Process()

#p.max_events = 1000
p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
rcclu = HpstrConf.Processor('rclu','RoryClusterCompareSeedAnaProcessor')
rcclu.parameters["debug"] = 0
rcclu.parameters["layer"] = options.layer
rcclu.parameters["module"] = options.module
rcclu.parameters["isMC"] = options.isMC
rcclu.parameters["doTrack"] = options.doTrack
rcclu.parameters["cut"] = options.cut
rcclu.parameters["fileName1"] = options.fileName1
rcclu.parameters["fileName2"] = options.fileName2

sequence = [rcclu]        

p.sequence = sequence

print("THE INPUT FILES ARE "+str(options.fileName1)+" AND "+str(options.fileName2))
p.input_files = root_files
p.output_files = [ana_file]

p.printProcess()

import HpstrConf
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename
root_file = [options.outFilename]

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()


p.run_mode = 0

# Library containing processors
p.add_library("libprocessors")


###############################
#          Processors         #
###############################
ts = HpstrConf.Processor('ts', 'TSProcessor')

###############################
#   Processor Configuration   #
###############################
#Event
ts.parameters["debug"] = 0
ts.parameters["tsCollLcio"]   = "TSBank"
ts.parameters["tsCollRoot"]   = "TSBank"

# Sequence which the processors will run.
p.sequence = [ts]

p.input_files= lcio_file
p.output_files = root_file

if (options.nevents > -1):
    p.max_events = options.nevents

p.printProcess()

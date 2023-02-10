import HpstrConf
import os
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename[0]
root_file = options.outFilename

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

p = HpstrConf.Process()
p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

#Set files to process
p.input_files = [lcio_file]
p.output_files = [root_file]

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

svtblana = HpstrConf.Processor('svtblana', 'SvtBl2DAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#SvtBl2D
svtblana.parameters["debug"] = 1
svtblana.parameters["rawSvtHitsColl"] = "SVTRawTrackerHits"
svtblana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/Svt2DBl.json'
svtblana.parameters["triggerBankColl"] = "TSBank"
svtblana.parameters["triggerBankCfg"] = os.environ['HPSTR_BASE']+'/analysis/selections/triggerSelection.json'

# Sequence which the processors will run.
p.sequence = [svtblana]

p.printProcess()

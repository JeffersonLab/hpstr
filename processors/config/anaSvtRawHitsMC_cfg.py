import HpstrConf
import sys
import os

# Use the input file to set the output file name
infile = sys.argv[1].strip()
outfile = '%s_rawSvtMC.root' % infile[:-5]

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()
p.run_mode = 1

#Set files to process
p.input_files=[infile]
p.output_files = [outfile]

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

svtrawmcana = HpstrConf.Processor('svtrawmcana', 'RawSvtHitMCAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#SvtBl2D
svtrawmcana.parameters["debug"] = 1
svtrawmcana.parameters["rawSvtHitsColl"] = "SVTRawTrackerHits"
svtrawmcana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/RawSvtHitMC.json'

#p.max_events = 1000

# Sequence which the processors will run.
p.sequence = [svtrawmcana]

p.printProcess()

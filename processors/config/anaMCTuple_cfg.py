import HpstrConf
import sys

# Use the input file to set the output file name
infile = sys.argv[1].strip()
outfile = '%s_anaMC.root' % infile[:-5]

print 'Input file: %s' % infile
print 'Output file: %s' % outfile

p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

mcana = HpstrConf.Processor('mcana', 'MCAnaProcessor')
# Sequence which the processors will run.
p.sequence = [mcana]

p.input_files=[infile]
p.output_files = [outfile]

#p.max_events = 1000

p.printProcess()

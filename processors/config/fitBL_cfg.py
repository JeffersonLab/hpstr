import HpstrConf
import baseConfig as base
import os



def timeSample_callback(options, opt, value, parser):
        setattr(parser.values, options.dest, value.split(','))

base.parser.add_argument("-x", '--xmin', type=int, dest="xmin", 
        help="Set threshold for xmin of iterative fit range", metavar="xmin", default="50")
base.parser.add_argument("-m", "--minStats", type=int, dest="minStats", 
        help="Minimum Statistics required per bin to perform fit", metavar="minStats", default="8500")
base.parser.add_argument("-p", "--nPoints", type=int, dest="nPoints", 
        help="Select number of points for second derivative.", metavar="nPoints", default="3")
base.parser.add_argument("-b", "--rebin", type=int, dest="rebin",
                help="rebin factor.", metavar="rebin", default="1")
base.parser.add_argument('-s', '--hybrid', nargs='+', type=str, dest="hybrid",default="", 
        help="Enter baseline<#><hybrid_name>")


options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename[0]
root_file = options.outFilename[0]

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

# Use the input file to set the output file name
histo_file = options.inFilename[0]
hybrid = options.hybrid
#if hybrid != "":
#    fit_file = '%s_%s_BaselineFit.root'%(histo_file[:-5],hybrid)
#else:
#    fit_file = '%s_all_sensors_BaselineFit.root'%(histo_file[:-5])

p = HpstrConf.Process()

p.run_mode = 2

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

fitBL = HpstrConf.Processor('fitBL', 'SvtBlFitHistoProcessor')

###############################
#   Processor Configuration   #
###############################
fitBL.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/SvtBlFits.json'
fitBL.parameters["hybrid"] = options.hybrid
fitBL.parameters["rebin"] = options.rebin
fitBL.parameters["nPoints"] = options.nPoints
fitBL.parameters["xmin"] = options.xmin
fitBL.parameters["minStats"] = options.minStats

# Sequence which the processors will run.
p.sequence = [fitBL]

p.input_files=[histo_file]
p.output_files = [root_file]

p.printProcess()

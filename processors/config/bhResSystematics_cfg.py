import HpstrConf
import sys
import baseConfig

parser = baseConfig.parser

parser.add_argument("-s", "--spec", type=str, dest="mass_spec",
                    help="Name of mass spectrum histogram.", metavar="mass_spec",
                    default="mass_tweak__p_tot_min_cut")
parser.add_argument("-m", "--mass", type=int, dest="mass_hypo",
                    help="Mass hypothesis in MeV.", metavar="mass_hypo", default=145)
parser.add_argument("-p", "--poly", type=int, dest="poly_order",
                    help="Polynomial order of background model.", metavar="poly_order", default=3)
parser.add_argument("-w", "--win", type=int, dest="win_factor",
                    help="Window factor for determining fit window size.", metavar="win_factor", default=11)
parser.add_argument("-M", "--bkg_model", type=int, dest="bkg_model", default="1",
                    help="The type of background fit model. 0 = Chebyshev; 1 = Exponential Chebyshev; 2 = Legendre; 3 = Exponential Legendre.", metavar="bkg_model")
parser.add_argument("-N", "--num_iterations", type=int, dest="num_iters",
                    help="Number of iterations to run.", metavar="num_iters", default=1000)
parser.add_argument("-S", "--res_sigma", type=float, dest="res_sigma",
                    help="Width of the Guassian distriubtion of the mass resolution.", metavar="res_sigma", default=0.05)
parser.add_argument("-f", "--func_file", type=str, dest="function_file",
                    help="The name of the file containing the mass resolution error parameterization function.", metavar="function_file", default="")
parser.add_argument("-F", "--func_name", type=str, dest="function_name",
                    help="The name of the function object that describes the mass resolution parameterization.", metavar="function_name", default="")
parser.add_argument("-u", "--num_toys", type=int, dest="num_toys",
                    help="Number of toys to generate.", metavar="num_toys", default=1000)
parser.add_argument("-U", "--num_toy_iterations", type=int, dest="toy_num_iters",
                    help="Number of iterations to run.", metavar="toy_num_iters", default=100)

options = parser.parse_args()

# Use the input file to set the output file name
histo_file = options.inFilename
mass_hypo = options.mass_hypo/1000.0
poly_order = options.poly_order
win_factor = options.win_factor
out_file = '%s/bhResSys_m%iw%ip%i.root' % (options.outDir, options.mass_hypo, win_factor, poly_order)

print('Input File: %s' % histo_file)
print('Output File: %s' % out_file)

p = HpstrConf.Process()

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

bhressys = HpstrConf.Processor('bhtoys', 'BhMassResSystematicsProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
bhressys.parameters["debug"] = 1
bhressys.parameters["seed"] = 0
bhressys.parameters["massSpectrum"] = options.mass_spec
bhressys.parameters["mass_hypo"] = mass_hypo
bhressys.parameters["poly_order"] = poly_order
bhressys.parameters["win_factor"] = win_factor
bhressys.parameters["bkg_model"] = options.bkg_model
bhressys.parameters["num_iters"] = options.num_iters
bhressys.parameters["res_sigma"] = options.res_sigma
bhressys.parameters["function_file"] = options.function_file
bhressys.parameters["function_name"] = options.function_name
bhressys.parameters["num_toys"] = options.num_toys
bhressys.parameters["toy_num_iters"] = options.toy_num_iters

# Sequence which the processors will run.
p.sequence = [bhressys]

p.input_files = [histo_file]
p.output_files = [out_file]

p.printProcess()

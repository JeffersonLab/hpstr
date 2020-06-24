import HpstrConf
import sys
import baseConfig as base

parser = base.parser

parser.add_argument("-m", "--mass", type=int, dest="mass_hypo",
        help="Mass hypothesis in MeV.", metavar="mass_hypo", default=145)
parser.add_argument("-p", "--poly", type=int, dest="poly_order",
        help="Polynomial order of background model.", metavar="poly_order", default=3)
parser.add_argument("-P", "--toy_poly", type=int, dest="toy_poly_order",
        help="Polynomial order of toy generator fit.", metavar="toy_poly_order", default=-1)
parser.add_argument("-w", "--win", type=int, dest="win_factor",
        help="Window factor for determining fit window size.", metavar="win_factor", default=11)
parser.add_argument("-N", "--toys", type=int, dest="nToys",
        help="Number of toy spectra to throw.", metavar="nToys", default=100)
parser.add_argument("-s", "--spec", type=str, dest="mass_spec",
        help="Name of mass spectrum histogram.", metavar="mass_spec",
        default="mass_tweak__p_tot_min_cut")
parser.add_argument("-a", "--sig", type=int, dest="toy_sig_samples",
        help="Number of signal events to add to toy models.", metavar="toy_sig_samples",
        default=0)
parser.add_argument("-f", "--sig_file", type=str, dest="signal_shape_h_file",
        help="Name of the file containing the signal shape injection histogram.", metavar="signal_shape_h_file", default="")
parser.add_argument("-g", "--sig_hist", type=str, dest="signal_shape_h_name",
        help="Name of the signal shape injection histogram.", metavar="signal_shape_h_name", default="")
parser.add_argument("-b", "--bkg", type=int, dest="toy_bkg_mult",
        help="Number of toy background events in units of the integral of the input distribution.",
        metavar="toy_bkg_mult", default=1)
parser.add_argument("-r", "--res_scale", type=float, dest="res_scale",
        help="Factor by which to scale the mass resolution.", metavar="res_scale", default=1.56)
parser.add_argument("-M", "--bkg_model", type=int, dest="bkg_model", default="1",
        help="The type of background fit model. 0 = Chebyshev; 1 = Exponential Chebyshev; 2 = Legendre; 3 = Exponential Legendre.", metavar="bkg_model")
options = parser.parse_args()

# Use the input file to set the output file name
histo_file = options.inFilename[0]
mass_hypo = options.mass_hypo/1000.0
poly_order = options.poly_order
win_factor = options.win_factor
toy_file = '%s/bhToys_m%iw%ip%ir%is%i.root'%(options.outDir, options.mass_hypo, win_factor, poly_order, int(options.res_scale * 100), options.toy_sig_samples)

print('Histo file: %s' % histo_file)
print('Toy file: %s' % toy_file)

p = HpstrConf.Process()

p.run_mode = 2
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

bhtoys = HpstrConf.Processor('bhtoys', 'BhToysHistoProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
bhtoys.parameters["debug"] = 1
bhtoys.parameters["massSpectrum"] = options.mass_spec
bhtoys.parameters["mass_hypo"] = mass_hypo
bhtoys.parameters["poly_order"] = poly_order
bhtoys.parameters["toy_poly_order"] = options.toy_poly_order
bhtoys.parameters["win_factor"] = win_factor
bhtoys.parameters["seed"] = 0
bhtoys.parameters["nToys"] = options.nToys
bhtoys.parameters["toy_sig_samples"] = options.toy_sig_samples
bhtoys.parameters["toy_bkg_mult"] = options.toy_bkg_mult
bhtoys.parameters["res_scale"] = options.res_scale
bhtoys.parameters["signal_shape_h_name"] = options.signal_shape_h_name
bhtoys.parameters["signal_shape_h_file"] = options.signal_shape_h_file
bhtoys.parameters["bkg_model"] = options.bkg_model

# Sequence which the processors will run.
p.sequence = [bhtoys]

p.input_files=[histo_file]
p.output_files = [toy_file]

p.printProcess()

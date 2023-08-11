#!/usr/bin/python3
import sys
sys.path.append( '/sdf/group/hps/users/alspellm/projects/THESIS/ana/plot_utils')
import my_plot_utils as utils
import ROOT as r

################################################################
def get_zbi_for_iter(zbis_hh, iteration):
    #Sig and Bkg plots for iteration have ZBi from previous iteration
    iteration = iteration - 1
    if iteration < 0:
        return 0,0.0

    cut = {}
    iter_bin = zbis_hh.GetXaxis().FindBin(iteration)
    projy = zbis_hh.ProjectionY("iteration_%i_zbi"%(iteration),iter_bin,iter_bin,"")
    zbi = projy.GetMean()
    zbi = round(zbi,2)
    cut_id = int(projy.Integral())
    cut[cut_id] = zbi
    return cut_id, zbi

def get_test_cut_values_for_iter(cuts_hh, iteration):
    iter_bin = cuts_hh.GetXaxis().FindBin(iteration)
    projy = cuts_hh.ProjectionY("iteration_%i_cuts"%(iteration),iter_bin,iter_bin,"")
    cuts_values = {}
    for x in range(projy.GetNbinsX()):
        if projy.GetXaxis().GetBinLabel(x+1) == "":
            continue
        cut = projy.GetXaxis().GetBinLabel(x+1)
        val = round(projy.GetBinContent(x+1),2)
        cuts_values[cut] = val
    return cuts_values

################################################################

r.gROOT.SetBatch(1)
colors = utils.getColors()

infileName = "~/work/projects/THESIS/ana/isolation_cut_dev/20230724_100MeV/cut_tests/zalpha_gt_lt/signal_55_bkg20_sig5_zalpha_slope_0.03_gt_lt.root"
plots_dir = 'zalpha_plots_html_test/1d_plots'
signal_1d_plots = utils.read_1d_plots_from_root_file_dirs(infileName,"signal","_h")
background_1d_plots = utils.read_1d_plots_from_root_file_dirs(infileName,"background","_h")
#print(signal_1d_plots.keys())

#Get persistent cuts
pers_cuts_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_persistent_cuts_hh")

#Get best cut ZBi from previous iteration
zbis_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_best_test_cut_ZBi_hh")

#Test Cut Values from previous iteration
testcuts_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_values_hh")

#Test Cut ZBis from previous iteration
testcuts_zbi_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_ZBi_hh")

#Test Cut Zcut
testcuts_zcut_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_zcut_hh")

#Test Cut Nsig
testcuts_nsig_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_nsig_hh")

#Test Cut nbkg
testcuts_nbkg_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_nbkg_hh")


#sys.exit()
for key, values in signal_1d_plots.items():
    if "initial" in key:
        iteration = "initial"
    else:
        iteration = int(key.split('_')[-1].split('.')[0])

    text = []
    '''
    if iteration != "initial" and iteration > 0:
        cut_values = get_cut_values_for_iter(pers_cuts_hh, iteration)
        best_cut_id, iter_zbi = get_zbi_for_iter(zbis_hh, iteration)
        if best_cut_id > 0:
            best_cut = cut_values[best_cut_id-1]
            cut_values.append("Best cut %s ZBi: %f"%(best_cut, iter_zbi))
        text = cut_values
    '''
    if iteration != "initial":
        pers_cuts = get_test_cut_values_for_iter(pers_cuts_hh, iteration)
        for cut_key, value in pers_cuts.items():
           line = "Active Cut %s %s"%(cut_key, str(value))
           text.append(line) 
        if iteration > 0:
            #Read test cut values from previous iteration, which correspond to the sig/bkg plots for this iteration
            #test_cuts = get_test_cut_values_for_iter(testcuts_hh, iteration-1)
            test_cuts_zbi = get_test_cut_values_for_iter(testcuts_zbi_hh, iteration-1)
            test_cuts_zcut = get_test_cut_values_for_iter(testcuts_zcut_hh, iteration-1)
            test_cuts_nsig = get_test_cut_values_for_iter(testcuts_nsig_hh, iteration-1)
            test_cuts_nbkg = get_test_cut_values_for_iter(testcuts_nbkg_hh, iteration-1)
            max_zbi = -999.9
            max_zbi_cut = ""
            for zbi_key, value in test_cuts_zbi.items():
                if value > max_zbi:
                    max_zbi = value
                    max_zbi_cut = zbi_key
            zbi = str(test_cuts_zbi[max_zbi_cut])
            zcut = str(test_cuts_zcut[max_zbi_cut])
            nsig = str(test_cuts_nsig[max_zbi_cut])
            nbkg = str(test_cuts_nbkg[max_zbi_cut])
            text.append("Iteration has peak zbi %s at zcut %s [mm]. Nsig=%s | Nbkg=%s"%(zbi, zcut, nsig, nbkg))

    for n,value in enumerate(values):
        signal_h = value
        if signal_h.GetEntries() < 1:
            continue
        signal_name = signal_h.GetName()
        var_name = signal_name.replace('signal_','')

        bkg_key = key.replace('signal','background')
        bkg_h = background_1d_plots[bkg_key][n]
        bkg_name = bkg_h.GetName()

        #format histos
        utils.format_TH1(signal_h,line_width = 2, line_color = colors[0])
        utils.format_TH1(bkg_h,line_width = 2, line_color = colors[1])

        canvas_name = 'iteration_%s_%s'%(str(iteration), var_name) 
        print(canvas_name)
        utils.plot_TH1s_with_legend([signal_h,bkg_h], canvas_name, plots_dir,LogY=True,insertText = text, text_x = 0.5, text_size=0.02)


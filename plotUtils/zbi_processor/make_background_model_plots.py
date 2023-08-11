#!/usr/bin/python3
import sys
sys.path.append( '/sdf/group/hps/users/alspellm/projects/THESIS/ana/plot_utils')
import my_plot_utils as utils
import ROOT as r

################################################################
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

def get_zbi_for_iter(zbis_hh, iteration):
    cut = {}
    iter_bin = zbis_hh.GetXaxis().FindBin(iteration)
    projy = zbis_hh.ProjectionY("iteration_%i_zbi"%(iteration),iter_bin,iter_bin,"")
    zbi = projy.GetMean()
    zbi = round(zbi,2)
    cut_id = int(projy.Integral())
    cut[cut_id] = zbi
    return cut_id, zbi


################################################################

r.gROOT.SetBatch(1)
colors = utils.getColors()

infileName = "~/work/projects/THESIS/ana/isolation_cut_dev/20230724_100MeV/cut_tests/zalpha_gt_lt/signal_55_bkg20_sig5_zalpha_slope_0.03_gt_lt.root"
plots_dir = 'zalpha_plots_html_test/background_model_plots'
plots = utils.read_1d_plots_from_root_file_dirs(infileName,"testCuts","background_zVtx")

#Get the test cuts
pers_cuts_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_persistent_cuts_hh")
#Zbis
zbis_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_best_test_cut_ZBi_hh")

#Test Cuts
testcuts_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_values_hh")
#Test Cut ZBis
testcuts_zbi_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_ZBi_hh")
#Test Cut Zcut
testcuts_zcut_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_zcut_hh")
#Test Cut Nsig
testcuts_nsig_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_nsig_hh")
#Test Cut nbkg
testcuts_nbkg_hh = utils.read_plot_from_root_file(infileName, "zbi_processor_test_cuts_nbkg_hh")

# Open the ROOT file
root_file = r.TFile.Open(infileName,"READ")

for key, values in plots.items():

    iteration = int(key.split('_')[-1].split('.')[0])
    test_cuts = get_test_cut_values_for_iter(testcuts_hh, iteration)
    test_cuts_zbi = get_test_cut_values_for_iter(testcuts_zbi_hh, iteration)
    test_cuts_zcut = get_test_cut_values_for_iter(testcuts_zcut_hh, iteration)
    test_cuts_nsig = get_test_cut_values_for_iter(testcuts_nsig_hh, iteration)
    test_cuts_nbkg = get_test_cut_values_for_iter(testcuts_nbkg_hh, iteration)
    text = []
    for key, value in test_cuts.items():
        zbi = str(test_cuts_zbi[key])
        zcut = str(test_cuts_zcut[key])
        nsig = str(test_cuts_nsig[key])
        nbkg = str(test_cuts_nbkg[key])
        line = "Test %s has %s [zbi] at zcut %s [mm]. Nsig=%s | Nbkg=%s"%(key, zbi, zcut, nsig, nbkg)
        text.append(line)

    print("Final text: ", text)

    histos = []
    for n, value in enumerate(values):
        h = value
        name = h.GetName()
        var_name = name.replace('testCutHistos_background_zVtx','').replace('_h','')
        h.SetName(var_name)
        h.SetTitle(var_name)
        utils.format_TH1(h,line_width = 2, line_color = colors[n])
        func_name = h.GetListOfFunctions().At(0).GetName()
        func = h.GetFunction("%s"%(func_name))
        func.SetLineColor(colors[n])
        histos.append(h)

    canvas_name = 'iteration_%s_test_cuts'%(str(iteration)) 
    print(canvas_name)
    utils.plot_TH1s_with_legend(histos, canvas_name, plots_dir,LogY=True,insertText = text, text_x = 0.4, text_size=0.02)
'''
#Loop over all directories matching key
dir_keyword = "testCuts"
keyword = "background_zVtx"
root_file.cd()
for dir_key in root_file.GetListOfKeys():
    dir_obj = dir_key.ReadObj()

    # Check if the object is TDir
    if isinstance(dir_obj, r.TDirectory):
        if dir_keyword not in dir_obj.GetName():
            continue

        # Get the directory within the ROOT file
        print("Navigating to directory ", dir_obj.GetName())
        root_dir = root_file.GetDirectory(dir_obj.GetName())

        iteration = int(dir_obj.GetName().split('_')[-1].split('.')[0])
        print("Iteration", iteration)
        text = []
        cut_values = get_cut_values_for_iter(pers_cuts_hh, iteration)
        best_cut_id, iter_zbi = get_zbi_for_iter(zbis_hh, iteration)
        if best_cut_id > 0:
            best_cut = cut_values[best_cut_id-1]
            cut_values.append("Best cut %s ZBi: %f"%(best_cut, iter_zbi))
        text = cut_values


        histos = []

        # Loop over all objects in the directory
        n = 0
        for key in root_dir.GetListOfKeys():
            h = key.ReadObj()

            # Check if the object is a 1D histogram
            if isinstance(h, r.TH1) and h.GetDimension() == 1:
                # Check if the object name contains the keyword (if provided)
                if keyword not in h.GetName():
                    continue

                name = h.GetName()
                var_name = name.replace('testCutHistos_background_zVtx','').replace('_h','')
                h.SetName(var_name)
                h.SetTitle(var_name)
                utils.format_TH1(h,line_width = 2, line_color = colors[n])
                histos.append(h)
                n = n + 1
        canvas_name = 'iteration_%s_test_cuts'%(str(iteration)) 
        print(canvas_name)
        utils.plot_TH1s_with_legend(histos, canvas_name, plots_dir,LogY=True,insertText = text)
'''


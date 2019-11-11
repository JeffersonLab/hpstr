import ROOT as r
import numpy as np
from copy import deepcopy
import utilities as utils
import SvtCondAnaTools as svtc



from optparse import OptionParser
parser=OptionParser()
parser.add_option("--id", dest="inputDir", help="inputDirectory",default="")
#parser.add_option("--inF",dest="inputFile",help="inputFile",default="")
#parser.add_option("--outF",dest="outputFile",help="outputFile",default="rmsPlots.root")
(options, args) = parser.parse_args()

in_path_to_File=""
run = options.inputDir[:-1]
in_file_ext="_anaSvtCond.root"
inFile = r.TFile(in_path_to_File+options.inputDir+run+in_file_ext)

out_path_to_File=""
out_file_ext="_testing.root"
outFileName=out_path_to_File+options.inputDir+run+out_file_ext


hybrid_names=svtc.BuildHybridNames()
hybridSample0=svtc.DeepCopy(inFile, hybrid_names)
scADCrms=svtc.GetRMS(hybridSample0)

#PNG_file_path="Images/"

#Dead vs Alive channel analysis
ratioAlive=[]
deadChannels=[]
avgRMS=[]
rmsofRMS=[]
for i in range(len(scADCrms)):
     if i<8:
        cmax=510
     else:
        cmax=640
     rtemp, dctemp = svtc.RatioDeadAlive(cmax, 150, scADCrms[i])
     ratioAlive.append(rtemp)
     deadChannels.append(dctemp)
     avgRMS.append(svtc.AvgRMS(scADCrms[i],cmax))
     rmsofRMS.append(svtc.RMSrms(scADCrms[i],cmax))
layer, module = svtc.swModuleMapper()
svtc.Histo2D(outFileName, "Alive Channels", "Percentage of Alive Channels for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, ratioAlive, "RECREATE", False)
svtc.Histo2D(outFileName, "Average RMS", "Averae RMS across Hybrid for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, avgRMS, "UPDATE", False)
svtc.Histo2D(outFileName, "RMS of RMS", "RMS of the RMS across Hybrid for "+run, "Layer", "Module", 14, 0.5, 14.5, 4, -0.5,3.5,layer, module, rmsofRMS, "UPDATE", False)




#Create Plots and Histograms for each Hybrid
for i in range(len(scADCrms)):
   # if i==0:
    #    update="RECREATE"
     #   end = False
    if i<len(scADCrms):
        update="UPDATE"
        end=False
    if i==len(scADCrms):
        end = True
    svtc.Histo1D(outFileName, hybrid_names[i]+"_h", hybrid_names[i]+"_"+run+"_h", "RMS [ADC Units]", "Number of Strips", 250, 0.5, 1000.5, scADCrms[i], update, end)
    svtc.Plot(outFileName, hybrid_names[i]+"_g", hybrid_names[i]+"_"+run+"_g", "Strip Number", "RMS [ADC Units]", 640, scADCrms[i], update, end)
    svtc.Histo1D(outFileName, "Dead_Channels_"+hybrid_names[i], "Dead Channels for "+hybrid_names[i]+run,"Channel", "Dead", 640, 0, 640, deadChannels[i],update, end)


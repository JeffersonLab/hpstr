from ROOT import *
import sys
sys.path.append("/Users/pbutti/sw/hpstr/plotUtils")

from optparse import OptionParser
import iterativeFitting as itf


class plotFitter:
    def __init__(self, dataFile, mcFile, outFile, regions):
        self.dataFile = dataFile
        self.mcFile   = mcFile
        self.outFile  = outFile
        self.regions  = regions
        self.sigmaRange = 1.5
        self.rangeFit = [1.7,3]
        self.rebinX   = 1
        self.rebinY   = 1
        self.hmap     = {}
        self.testFactor = 1.1   # Only for testing purposes when MC is missing

    def writeHistoMap(self):
        self.outFile.cd()
        for hname in self.hmap.keys():
            self.hmap[hname].Write()
        

    def smearing_term(self, sigma_data, sigma_mc, sigma_dataerr=0., sigma_mcerr=0.):
        
        st = -1.
        sigma_a = sigma_data
        sigma_b = sigma_mc
        
        if (sigma_data < sigma_mc):
            return (0.,0.)
        
        st = sqrt(sigma_data*sigma_data - sigma_mc*sigma_mc)

        if (st < 1e-6):
            return (0.,0.)
        
        dstdsd = sigma_data / st
        dstdsmc = -sigma_mc / st

        sstsd2  = dstdsd*dstdsd*sigma_dataerr*sigma_dataerr
        sstsmc2 = dstdsmc*dstdsmc*sigma_mcerr*sigma_mcerr
        
        sigmast = sqrt(sstsd2 + sstsmc2)

        #Use 1% for errors very small. For testing purposes.
        if (sigmast < 1e-6):
            sigmast = st * 0.01
            
        return st,sigmast

    def fit1D(self,hname):

        hdata = self.dataFile.Get(hname)
        hmc   = self.mcFile.Get(hname)
        if not hdata:
            print("ERROR::",hname," not found in ", self.dataFile)
        if not hmc:
            print("ERROR::",hname," not found in ", self.mcFile)
            

        #Perform the gaussian recursive fit and extract mean and sigma
        
        datafit = itf.singleGausIterative(hdata,
                                          self.sigmaRange,
                                          self.rangeFit)
        
        mcfit = itf.singleGausIterative(hmc,
                                        self.sigmaRange,
                                        self.rangeFit)
        
        mu_data    = datafit.GetParameter(1)
        sigma_data = datafit.GetParameter(2)

        sigma_data = sigma_data * self.testFactor
        
        mu_mc      = mcfit.GetParameter(1)
        sigma_mc   = mcfit.GetParameter(2)
        
        st,sigmast = self.smearing_term(sigma_data,sigma_mc)

        self.hmap[hname+"_smearing"] = TH1F(hname+"_smearing",
                                            hname+"_smearing",
                                            1,0,1)

        self.hmap[hname+"_smearing"].SetBinContent(1,st)
        self.hmap[hname+"_smearing"].SetBinError(1,sigmast)
        
        return st
    

    def fit2D(self,hname):
        
        hdata = self.dataFile.Get(hname)
        hmc   = self.mcFile.Get(hname)

        if not hdata:
            print("ERROR::",hname," not found in ", self.dataFile)
            return 
        if not hmc:
            print("ERROR::",hname," not found in ", self.mcFile)
            return 
        

        hdata.RebinX(self.rebinX)
        hmc.RebinX(self.rebinX)

        self.hmap[hname+"_smearing"] = TH1F(hdata.GetName()+"_smearing",
                                            hdata.GetName()+"_smearing",
                                            hdata.GetXaxis().GetNbins(),
                                            hdata.GetXaxis().GetXmin(),
                                            hdata.GetXaxis().GetXmax())
        
    
        h_mu_data    = TH1F(hdata.GetName()+"_mu_data",
                            hdata.GetName()+"_mu_data",
                            hdata.GetXaxis().GetNbins(),
                            hdata.GetXaxis().GetXmin(),
                            hdata.GetXaxis().GetXmax())
        
        h_sigma_data = TH1F(hdata.GetName()+"_sigma_data",
                            hdata.GetName()+"_sigma_data",
                            hdata.GetXaxis().GetNbins(),
                            hdata.GetXaxis().GetXmin(),
                            hdata.GetXaxis().GetXmax())
        
        h_mu_mc      = TH1F(hdata.GetName()+"_mu_mc",
                            hdata.GetName()+"_mu_mc",
                            hdata.GetXaxis().GetNbins(),
                            hdata.GetXaxis().GetXmin(),
                            hdata.GetXaxis().GetXmax())
        
        h_sigma_mc   = TH1F(hdata.GetName()+"_sigma_mc",
                            hdata.GetName()+"_sigma_mc",
                            hdata.GetXaxis().GetNbins(),
                            hdata.GetXaxis().GetXmin(),
                            hdata.GetXaxis().GetXmax())
        

        itf.ProfileYwithIterativeGaussFit(hdata,
                                          h_mu_data,
                                          h_sigma_data,
                                          1, #do not rebin internally
                                          fitrange=self.rangeFit)

        
        itf.ProfileYwithIterativeGaussFit(hmc,
                                          h_mu_mc,
                                          h_sigma_mc,
                                          1, #do not rebin internally
                                          fitrange=self.rangeFit)
        
        #Now compute the smearing factor for each bin
        
        for ibin in range(h_sigma_data.GetXaxis().GetNbins()):
            sigma_data = h_sigma_data.GetBinContent(ibin)*self.testFactor
            sigma_mc   = h_sigma_mc.GetBinContent(ibin)
            st,sigmast = self.smearing_term(sigma_data,sigma_mc)

            self.hmap[hname+"_smearing"].SetBinContent(ibin,st)
            self.hmap[hname+"_smearing"].SetBinError(ibin,sigmast)
            
            pass

    def fit3D(self,hname):

        hdata  = self.dataFile.Get(hname)
        hmc    = self.mcFile.Get(hname)

        if not hdata:
            print("ERROR::",hname," not found in ", self.dataFile)
            return 
        if not hmc:
            print("ERROR::",hname," not found in ", self.mcFile)
            return

        hdata.RebinX(self.rebinX)
        hdata.RebinY(self.rebinY)
        hmc.RebinX(self.rebinX)
        hmc.RebinY(self.rebinY)

        hmudata,hsigmadata,hmuerrdata,hsigmaerrdata = itf.profileZwithCustomFit(hdata,
                                                                                1,
                                                                                self.rangeFit,
                                                                                name="_data")


        hmumc,hsigmamc,hmuerrmc,hsigmaerrmc = itf.profileZwithCustomFit(hmc,
                                                                        1,
                                                                        self.rangeFit,
                                                                        name="_mc")
        

        nbinsx = hdata.GetXaxis().GetNbins()
        nbinsy = hdata.GetYaxis().GetNbins()
        
        self.hmap[hname+"_smearing"] = TH2F(hname+"_smearing",
                                            hname+"_smearing",
                                            nbinsx,
                                            hdata.GetXaxis().GetXmin(),
                                            hdata.GetXaxis().GetXmax(),
                                            nbinsy,
                                            hdata.GetYaxis().GetXmin(),
                                            hdata.GetYaxis().GetXmax())


        for ibinX in range(nbinsx):
            for ibinY in range(nbinsy):
                sigma_data = hsigmadata.GetBinContent(ibinX,ibinY)*self.testFactor
                sigma_data_err = hsigmaerrdata.GetBinContent(ibinX,ibinY)

                sigma_mc   = hsigmamc.GetBinContent(ibinX,ibinY)
                sigma_mc_err = hsigmaerrmc.GetBinContent(ibinX,ibinY)

                st,sigmast = self.smearing_term(sigma_data,sigma_mc)
                self.hmap[hname+"_smearing"].SetBinContent(ibinX,ibinY,st)
                self.hmap[hname+"_smearing"].SetBinError(ibinX,ibinY,sigmast)
        
        
        
def main():

    parser = OptionParser()

    parser.add_option("-i","--infile",type="string",dest="infile",
                      help="Input root file", default="in.root")
    parser.add_option("-m","--mc",type="string",dest="mc",
                      help="Input mc file", default="mc.root")
    parser.add_option("-o","--outfile",type="string",dest="outfile",
                      help="Output root file", default="out.root")

    parser.add_option("-r","--regions", type="string", dest="regions",help="Comma separated list of regions. If using \"all\" then loop on all the regions in the file", default="all")

    

    (options,args) = parser.parse_args()
    
    dataFile = TFile(options.infile)
    mcFile   = TFile(options.infile)
    outFile= TFile(options.outfile,"RECREATE")

    print(options.regions)
    
    regions = []
    if (options.regions == "all" or options.regions == "ALL"):
        keys = dataFile.GetListOfKeys()
        for key in keys:
            keyname = key.GetName()
            if ("event" not in keyname):
                regions.append(keyname)
    else:
        regions = regions.split(",")


    print(regions)
    
    
    pf = plotFitter(dataFile, mcFile, outFile, regions)
    
    smearing_term = pf.fit1D("KalmanFullTracks/KalmanFullTracks_p_h")
    
            
    pf.fit2D("KalmanFullTracks/KalmanFullTracks_p_vs_TanLambda_hh")
    pf.fit2D("KalmanFullTracks/KalmanFullTracks_p_vs_Phi_hh")
    pf.fit3D("KalmanFullTracks/KalmanFullTracks_p_vs_TanLambda_Phi_hhh")
    
    pf.writeHistoMap()
    
    pf.outFile.Close()

if __name__=="__main__":
    main()

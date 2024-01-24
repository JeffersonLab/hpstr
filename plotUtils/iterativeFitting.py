from ROOT import *
from array import array

def MakeFit(histoGram, fitType, markerColor,fitrange=[-2e5,2e5],sigmarange=2):

    #make sure the styles are integers
    markerColor = int(markerColor)

    #no Fit
    if fitType=="noFit":
        return None
    elif fitType=="singleGausIterative":
        fit = singleGausIterative(histoGram, sigmarange)

    fit.SetLineColor(markerColor)
    
    return fit


def ProfileYwithIterativeGaussFit(hist, mu_graph, sigma_graph, num_bins,fitrange=[-2e5,2e5],error_scaling=1.):
    
    if (num_bins < 1):
        return 

    minEntries = 50
    fDebug = False
    
    num_bins_x = hist.GetXaxis().GetNbins()
    mu_graph.Rebin(num_bins)
    sigma_graph.Rebin(num_bins)


    f = (num_bins_x / num_bins + 2)
    errs_mu    = [0. for x in range (int(num_bins_x / num_bins + 2))]
    errs_sigma = [0. for x in range (int(num_bins_x / num_bins + 2))]

    min_sigma = 0.
    max_sigma = 0.
    min_mu = 0.
    max_mu = 0.
    
    num_skipped = 0 
    
    current_proj = None
    
    for i in range(1,num_bins_x+1, num_bins):
        index = int(i / num_bins)
        if (num_bins == 1):
            index -= 1 
        
        
        current_proj = hist.ProjectionY(hist.GetName()+"_"+str(index),i,i+num_bins-1)
        
        mu = 0.
        mu_err = 0.
        sigma = 0.
        sigma_err = 0.
        fit = None

        if (current_proj.GetEntries() < minEntries):
            continue
        else:
            fit = singleGausIterative(current_proj,2,fitrange)
            
        mu = fit.GetParameter(1)
        mu_err = fit.GetParError(1)
        
        sigma  = fit.GetParameter(2)
        sigma_err = fit.GetParError(2)


        singlePlots=False
        if (singlePlots):
            c = TCanvas()
            c.cd()
            current_proj.Draw("p")
            fit.Draw("same")
            c.SaveAs(current_proj.GetName() + ".pdf")
        
        
        if (sigma > max_sigma or max_sigma == 0):
            max_sigma = sigma
        if (sigma < min_sigma or min_sigma == 0):
            min_sigma = sigma

        if (mu > max_mu or max_mu == 0) :
            max_mu = mu
        if (mu < min_mu or min_mu == 0) :
            min_mu = mu
            
        value_x = (hist.GetXaxis().GetBinLowEdge(i) + hist.GetXaxis().GetBinUpEdge(i+num_bins-1))/2.
        
        #Important!! Use Fill to increment the graph with each iteration, or SetBinContent to replace contents...
        if (sigma_graph!=None):
            sigma_graph.Fill(value_x, sigma);
            
        if (mu_graph!=None):
            mu_graph.Fill(value_x, mu);
            
        errs_mu[index+1] = mu_err * error_scaling
        errs_sigma[index+1 ] = sigma_err * error_scaling
        

    a_errs_mu = array("d", errs_mu)
    a_errs_sigma = array("d",errs_sigma)
    if (sigma_graph != None):
        sigma_graph.SetError(a_errs_sigma)
        sigma_graph.GetYaxis().SetTitleOffset(1.5)
        sigma_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        sigma_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        sigma_graph.SetTitle("")
        
    if (mu_graph != None):
        mu_graph.SetError(a_errs_mu)
        mu_graph.GetYaxis().SetTitleOffset(1.5)
        mu_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        mu_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        mu_graph.SetTitle("")

    if (fDebug and num_skipped):
        print("Number of skipped bins: ", num_skipped)


#This fit requires some initial parameters guess to be reliable
def CrystalBallFit(hist, initialParameters, fitrange=[]):
     debug = False
    
     if len(fitrange) ==0:
         fitrange.append(histo.GetXaxis().GetXmin())
         fitrange.append(histo.GetXaxis().GetXmax())
    
     CrystalFit = TF1("CrystalFit","[0] * ROOT::Math::crystalball_function(x,[1],[2],[3],[4])",fitrange[0],fitrange[1])

     #https://en.wikipedia.org/wiki/Crystal_Ball_function#/media/File:CrystalBallFunction.svg
     CrystalFit.SetParameter(0,initialParameters[0])
     CrystalFit.SetParameter(1,1)  #alpha - 1 is a reasonable choice
     CrystalFit.SetParameter(2,3)  #n - 3 is a reasonable choice
     CrystalFit.SetParameter(3,initialParameters[2])   #sigma
     CrystalFit.SetParameter(4,initialParameters[1])   #mu

     
     hist.Fit("CrystalFit","ORQN","same")


     #c = TCanvas(hist.GetName())
     #hist.Draw()
     #CrystalFit.Draw("same")
     #c.SaveAs("./"+hist.GetName()+".pdf")
     
     return CrystalFit

 
def singleGausIterative(hist, sigmaRange,rangeFit=[]):
     debug = False
     # first perform a single Gaus fit across full range of histogram or in a specified range
     
     min = hist.GetBinLowEdge(1)
     max = (hist.GetBinLowEdge(hist.GetNbinsX()))+hist.GetBinWidth(hist.GetNbinsX())
     userRange = False
     minUser = -2e5
     maxUser = 2e5
     
     if (len(rangeFit) != 0):
         userRange = True
         minUser = rangeFit[0]
         maxUser = rangeFit[1]
         min = minUser
         max = maxUser

     fitA = TF1("fitA", "gaus", min,max)
     hist.Fit("fitA","ORQN","same")
     fitAMean = fitA.GetParameter(1)
     fitASig = fitA.GetParameter(2)
 
     # performs a second fit with range determined by first fit
     max = fitAMean + (fitASig*sigmaRange)
     min = fitAMean - (fitASig*sigmaRange)
     if (userRange):
        if (max > maxUser):
            max = maxUser
        if (min < minUser):
            min = minUser
    
     fitB = TF1("fitB", "gaus", min,max)
     hist.Fit("fitB","ORQN","same")
     fitMean = fitB.GetParameter(1)
     fitSig = fitB.GetParameter(2)
     
     newFitSig = 99999
     newFitMean = 99999
     i = 0
     max = fitMean + (fitSig*sigmaRange)
     min = fitMean - (fitSig*sigmaRange)
     if (userRange):
        if (max > maxUser):
            max = maxUser
        if (min < minUser):
            min = minUser

     
     fit = TF1("fit", "gaus", min,max)
 
     while abs(fitSig - newFitSig) > 0.0005 or abs(fitMean - newFitMean) > 0.0005:
         
         if(i > 0):
             fitMean = newFitMean
             fitSig = newFitSig
         #print("i = ",i," fitMean = ",fitMean," fitSig = ",fitSig)
         max = fitMean + (fitSig*sigmaRange)
         min = fitMean - (fitSig*sigmaRange)
         if (userRange):
             if (max > maxUser):
                 max = maxUser
             if (min < minUser):
                 min = minUser
         
         fit.SetRange(min,max)
         hist.Fit("fit","ORQN","same")
         newFitMean = fit.GetParameter(1)
         newFitSig = fit.GetParameter(2)
         #print("i = ",i," newFitMean = ", newFitMean, " newFitSig = ",newFitSig)
         if(i > 50):
             if debug:
                 print("WARNING terminate iterative gaus fit because of convergence problems")
                 print("final mean =  ", newFitMean, ", previous iter mean = ", fitMean)
                 print("final sigma =  ", newFitSig, ", previous iter sigma = ", fitSig)
             break
 
         i = i + 1
 
 
 
     if debug:
         print("Final i = ",i," finalFitMean = ", fit.GetParameter(1), " finalFitSig = ",fit.GetParameter(2))
 
     fit.SetLineWidth(2)
     
     return fit


#transform = 0: do nothing
#transform = 1: flip x
#transform = 2: flip y
#transform = 3: flip y and bin content
#transform = 4: flip x and y and bin content
def profileZwithCustomFit(hist,num_bins,fitrange=[-2e5,2e5],fittype="gaus",transform=0,name="") :

    
    if ( not hist):
        print("ProfileZwithIterativeGaussFit(): No histogram supplied!")
        return

    num_bins_x = hist.GetXaxis().GetNbins();
    num_bins_y = hist.GetYaxis().GetNbins();
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    
    mu_name          = hist.GetName() + "_mu_profiled_"+fittype+name
    mu_graph         = TH2D(mu_name,mu_name,num_bins_x, xmin, xmax,num_bins_y, ymin,ymax)
    sigma_name       = hist.GetName() + "_sigma_profiled_"+fittype+name
    sigma_graph      = TH2D(sigma_name,sigma_name,num_bins_x, xmin, xmax,num_bins_y, ymin,ymax)
    mu_err_name      = hist.GetName() + "_mu_err_profiled_"+fittype+name
    mu_err_graph     = TH2D(mu_err_name,mu_err_name,num_bins_x, xmin, xmax,num_bins_y, ymin,ymax)
    sigma_err_name   = hist.GetName() + "_sigma_err_profiled_"+fittype+name
    sigma_err_graph  = TH2D(sigma_err_name,sigma_err_name,num_bins_x, xmin, xmax,num_bins_y, ymin,ymax)


    
    mu_err = None
    sigma_err = None
    
    minEntries = 50;
    fDebug = 0;
     
    num_not_converged = 0;
    num_skipped = 0;
 
    max_sigma = 0;
    min_sigma = 0;
 
    max_mu = 0;
    min_mu = 0;
 
    current_proj = None
        
    for i in range(1,num_bins_x+(num_bins==1),num_bins):
        for j in range(1,num_bins_y+(num_bins==1),num_bins):
                        
            index   = i/num_bins;
            index_y = j/num_bins;
 
            current_proj = hist.ProjectionZ(hist.GetName()+"_"+str(index)+"_"+str(index_y),i,i+num_bins-1,j,j+num_bins-1)
            current_proj.SetTitle(hist.GetName()+"_"+str(index)+"_"+str(index_y));
           
            current_mu = -999
            current_err_mu = -999
            current_sigma = -999
            current_err_sigma = -999

            
            if current_proj.GetEntries() < minEntries :
                            
                current_mu = 0;
                current_sigma = 0;
                current_err_mu = 1;
                current_err_sigma = 1;
        
                if (fDebug):
                    print("WARNING: Only "+current_proj.GetEntries()+" entries in bin "+index+","+index_y+ " in histogram " +hist.GetName())
                    num_skipped+=1
                    
                    
            
            else:


                fit = singleGausIterative(current_proj,2,fitrange)
                current_norm = fit.GetParameter(0)
                current_mu = fit.GetParameter(1)
                current_err_mu = fit.GetParError(1)
                current_sigma = fit.GetParameter(2)
                current_err_sigma = fit.GetParError(2)


                #At thit point one can run a crystalBall

                if fittype=="cb":
                    crystalFit = CrystalBallFit(current_proj,
                                                [current_norm, current_mu, current_sigma],
                                                fitrange = [current_mu - 5*current_sigma, current_mu + 5*current_sigma])
                    
                    current_mu         = crystalFit.GetParameter(4)
                    current_err_mu     = crystalFit.GetParError(4)
                    current_sigma      = crystalFit.GetParameter(3)
                    current_err_sigma  = crystalFit.GetParError(3)
                    
                
                if (current_sigma > max_sigma or max_sigma == 0):
                    max_sigma = current_sigma
                if (current_sigma < min_sigma or min_sigma == 0):
                    min_sigma = current_sigma
                if (current_mu > max_mu or max_mu == 0):
                    max_mu = current_mu
                if (current_mu < min_mu or min_mu == 0):
                    min_mu = current_mu
 
                x_coord = (hist.GetXaxis().GetBinLowEdge(i) + hist.GetXaxis().GetBinUpEdge(i+num_bins-1))/2;
                y_coord = (hist.GetYaxis().GetBinLowEdge(j) + hist.GetYaxis().GetBinUpEdge(j+num_bins-1))/2;
 
                if (sigma_graph):
                    sigma_graph.Fill(x_coord,y_coord,current_sigma)
                if (mu_graph):
                    
                    if transform == 0:
                        mu_graph.Fill(x_coord,y_coord,current_mu)
                    elif transform == 1:
                        mu_graph.Fill(-x_coord,y_coord,current_mu)
                    elif transform == 2:
                        mu_graph.Fill(x_coord,-y_coord,current_mu)
                    elif transform == 3:
                        mu_graph.Fill(x_coord,-y_coord,-current_mu)
                    elif transform == 4:
                        mu_graph.Fill(-x_coord,-y_coord,-current_mu)
                             
                #should probably be replace bin content, not fill?
                if (sigma_err_graph):
                    sigma_err_graph.Fill(x_coord,y_coord,current_err_sigma);
                if (mu_err_graph):
                    mu_err_graph.Fill(x_coord,y_coord,current_err_mu);

                
    if (mu_graph) :
        mu_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        mu_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        mu_graph.GetYaxis().SetTitleOffset(1)
        mu_graph.GetZaxis().SetTitle(hist.GetZaxis().GetTitle())
        mu_graph.GetZaxis().SetTitleOffset(1.2)
        mu_graph.SetTitle( "" )
    
    if (sigma_graph) :
        sigma_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        sigma_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        sigma_graph.GetYaxis().SetTitleOffset(1)
        sigma_graph.GetZaxis().SetTitle(hist.GetZaxis().GetTitle())
        sigma_graph.GetZaxis().SetTitleOffset(1.2)
        sigma_graph.SetTitle( "" )
        
        
 
    if (mu_err_graph) :
        mu_err_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        mu_err_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        mu_err_graph.GetYaxis().SetTitleOffset(1)
        mu_err_graph.GetZaxis().SetTitle("Error of fit #mu:" + hist.GetZaxis().GetTitle())
        mu_err_graph.GetZaxis().SetTitleOffset(1.2)
        mu_err_graph.SetTitle(hist.GetTitle())
        

    if (sigma_err_graph):
        sigma_err_graph.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
        sigma_err_graph.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
        sigma_err_graph.GetYaxis().SetTitleOffset(1)
        sigma_err_graph.GetZaxis().SetTitle("Error of fit #sigma: "+hist.GetZaxis().GetTitle())
        sigma_err_graph.GetZaxis().SetTitleOffset(1.2)
        sigma_err_graph.SetTitle(hist.GetTitle())

    return mu_graph,sigma_graph,mu_err_graph,sigma_err_graph
    
       
                                
 





 

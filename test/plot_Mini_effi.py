'''
Usage:python plot.py RootFile.root label[optional]

Script to make some quick efficiency plots to test ntuple generation.


Author: L. Dodd, UW Madison

'''

from subprocess import Popen
from sys import argv, exit, stdout, stderr
import os
import ROOT
import array
# So things don't look like crap.
ROOT.gROOT.SetStyle("Plain")
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)
#ROOT.gStyle.SetTitleSize(0.1)
ROOT.gStyle.SetTitleBorderSize(0)
#ROOT.gStyle.SetPadTopMargin(0.1)
ROOT.gStyle.SetPadGridY(1)
ROOT.gStyle.SetTitleX(0.1)
ROOT.gStyle.SetTitleY(0.97)
#ROOT.gStyle.SetTitleW(0.6)
colors=[ROOT.kCyan,ROOT.kGreen,ROOT.kOrange,ROOT.kRed,ROOT.kBlue]  #Add more colors for >5 sets of points in a single plot
styles=[20,21,22,23,33]  #Add more point styles if needed

######## File #########
if len(argv) < 2:
   print 'Usage:python plot.py RootFile.root label[optional]'
   exit()

infile = argv[1]
ntuple_file = ROOT.TFile(infile)

######## LABEL & SAVE WHERE #########
if len(argv)>2:
   saveWhere='~/private/output/tauAnalysis/8_0_10/ZtoTT/Efficiency/'+argv[2]+'_'
else:
   saveWhere='~/private/output/tauAnalysis/8_0_10/ZtoTT/Efficiency/'

#####################################
#Get Effi NTUPLE                 #
#####################################
byLooseCmbIso3 = ntuple_file.Get("byLooseCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
byMedCmbIso3 = ntuple_file.Get("byMediumCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
byTightCmbIso3 = ntuple_file.Get("byTightCombinedIsolationDeltaBetaCorr3Hits/Ntuple")
ntrlIsoPtSum = ntuple_file.Get("neutralIsoPtSum/Ntuple")
puCorrPtSum = ntuple_file.Get("puCorrPtSum/Ntuple")
MuLoose3 = ntuple_file.Get("againstMuonLoose3/Ntuple")
MuTight3 = ntuple_file.Get("againstMuonTight3/Ntuple")
EleVLooseMVA6 = ntuple_file.Get("againstElectronVLooseMVA6/Ntuple")
EleLooseMVA6 = ntuple_file.Get("againstElectronLooseMVA6/Ntuple")
EleMediumMVA6 = ntuple_file.Get("againstElectronMediumMVA6/Ntuple")
EleTightMVA6 = ntuple_file.Get("againstElectronTightMVA6/Ntuple")
EleVTightMVA6 = ntuple_file.Get("againstElectronVTightMVA6/Ntuple")
canvas = ROOT.TCanvas("asdf", "adsf", 1200, 800)

def make_plot(tree, variable, selection, binning, title=''):
    ''' Plot a variable using draw and return the histogram '''
    draw_string = "%s>>htemp(%s)" % (variable, ", ".join(str(x) for x in binning))
    tree.Draw(draw_string, selection, "goff")
    output_histo = ROOT.gDirectory.Get("htemp").Clone()
    if variable == '':#tauPt
	inclusiveBinning = array.array('d', [20,25,30,35,40,45,50,55,60,65,70,80,90,100,120])
	output_histo = output_histo.Rebin(14,"rebinned",inclusiveBinning)

    return output_histo

def make_efficiency(denom, num):
    ''' Make an efficiency graph with the style '''
    eff = ROOT.TGraphAsymmErrors(num, denom)
    eff.SetMarkerStyle(20)
    eff.SetMarkerSize(2.0)
    eff.SetLineColor(ROOT.kBlack)
    return eff

def make_num(ntuple, variable,PtCut,binning):
    num = make_plot(
        ntuple, variable,
	"goodReco",
        binning
    )
    return num

def make_denom(ntuple, variable,PtCut,binning):
    denom = make_plot(
        ntuple, variable,
        "",
        binning
    )
    return denom

def produce_efficiency(ntuple, variable, PtCut,binning, filename,color,style):
    denom = make_denom(ntuple, variable,PtCut,binning)
    num = make_num(ntuple,variable,PtCut,binning)
    l1 = make_efficiency(denom,num)
    l1.SetMarkerColor(color)
    l1.SetMarkerStyle(style)
    
    return l1

#Accepts any number of ntuples and plots them for comparison; works well up to at least 5
def NewCompareEfficiencies(ntuples, legends, variable, PtCut, binning, filename, framemin, framemax, title, xaxis,yaxis):
	frame = ROOT.TH1F("frame", "frame", *binning)
	histlist = []
	for i in range(len(ntuples)):
		histlist.append(produce_efficiency(ntuples[i],variable, PtCut,binning, filename,colors[i],styles[i]))
	#frame.SetMaximum(framemax)
    	#frame.SetMinimum(framemin)
	#frame.SetTitle('CMS #it{Simulation} Z/#gamma*#rightarrow#tau#tau CMSSW 8_0_10')
	frame.SetTitle('CMS #it{Simulation} Z*#rightarrow#tau#tau CMSSW 8_0_10')
    	frame.GetYaxis().SetTitle(yaxis)
    	frame.GetXaxis().SetTitle(xaxis)
	frame.GetXaxis().SetTitleSize(0.3)
	frame.GetYaxis().SetTitleSize(0.3)
    	frame.GetXaxis().CenterTitle()
    	frame.GetYaxis().CenterTitle()
        #frame.SetNDC(kTRUE)
    	frame.UseCurrentStyle()
    	frame.Draw()
	for i in range(len(histlist)):
		if i == 0:
			histlist[i].Draw('pe')
		else:
			histlist[i].Draw('pesame')
    	legend = ROOT.TLegend(0.5,0.1 ,0.9,0.1 + 0.05*len(legends), "", "brNDC")
	if variable == 'dmf':
		text = ROOT.TPaveText(0.1,0.3,2,0.4)
		text.AddText("h^{#pm}")
		text.SetLineStyle(0)
		text.SetFillColor(0)
		text2 = ROOT.TPaveText(1,0.55,3,0.65)
		text2.AddText("h^{#pm}#pi^{0}")
		text2.SetLineStyle(0)
		text2.SetFillColor(0)
		text3 = ROOT.TPaveText(9,0.75,11,0.85)
		text3.AddText("h^{#pm}h^{#mp}h^{#pm}")
		text3.SetLineStyle(0)
		text3.SetFillColor(0)
		text.Draw()
		text2.Draw()
		text3.Draw()
    	legend.SetFillColor(ROOT.kWhite)
    	legend.SetBorderSize(1)
	for i in range(len(legends)):
		legend.AddEntry(histlist[i],legends[i],'pe')
    	legend.Draw()
    	saveas = saveWhere+filename+'.png'
    	print saveas
    	canvas.SaveAs(saveas)

################################################################################
# Efficiency for a 20 GeV cut on tau Pt 
################################################################################
## pT plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'tauPt', 20, [24,20,500],#variable, ptcut, binning
                    'tau_iso_effi_pT', 0, 1,#filename, lower bound (y), upper bound (y)
                    "Expected #tau efficiency",#title
                    "#tau_{h} p_{T} (GeV)",#xaxis
                    "Expected #tau efficiency" #yaxis            
)

NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'tauPt',20,[24,20,500],
                    'tau_PtSum_effi_pT', 0, 1,
                    "Expected #tau efficiency",
                    "#tau_{h} p_{T} (GeV)",
                    "Expected #tau efficiency"
)

NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'tauPt',20,[24,20,500],
                    'tau_Mu_effi_pT', 0, 1,
                    "Expected #tau efficiency",
                    "#tau_{h} p_{T} (GeV)",
                    "Expected #tau efficiency"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'tauPt',20,[24,20,500],
                    'tau_Ele_effi_pT', 0, 1,
                    "Expected #tau efficiency",
                    "#tau_{h} p_{T} (GeV)",
                    "Expected #tau efficiency"
)

## eta plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'tauEta', 20, [20,-2.4,2.4],#variable, ptcut, binning
                    'tau_iso_effi_eta', 0, 1,#filename, lower bound (y), upper bound (y)
                    "Expected #tau efficiency",#title
                    "Tau #eta",#xaxis
                    "Expected #tau efficiency" #yaxis             
)
NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'tauEta',20,[20,-2.4,2.4],
                    'tau_PtSum_effi_eta',0, 1,
                    "Expected #tau efficiency",
                    "Tau #eta",
                    "Expected #tau efficiency"
)
NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'tauEta',20,[20,-2.4,2.4],
                    'tau_Mu_effi_eta',0, 1,
                    "Expected #tau efficiency",
                    "Tau #eta",
                    "Expected #tau efficiency"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'tauEta',20,[20,-2.4,2.4],
                    'tau_Ele_effi_eta',0, 1,
                    "Expected #tau efficiency",
                    "Tau #eta",
                    "Expected #tau efficiency"
)

## nvtx plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'nvtx', 20, [20,0,35],#variable, ptcut, binning
                    'tau_iso_effi_nvtx',0, 1,#filename, lower bound (y), upper bound (y)
                    "Expected #tau efficiency",#title
                    "N_{vtx}",#xaxis
                    "Expected #tau efficiency" #yaxis             
)

NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'nvtx',20,[20,0,35],
                    'tau_PtSum_effi_nvtx',0, 1,
                    "Expected #tau efficiency",
                    "N_{vtx}",
                    "Expected #tau efficiency"
)

NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'nvtx',20,[20,0,35],
                    'tau_Mu_effi_nvtx',0, 1,
                    "Expected #tau efficiency",
                    "N_{vtx}",
                    "Expected #tau efficiency"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'nvtx',20,[20,0,35],
                    'tau_Ele_effi_nvtx',0, 1,
                    "Expected #tau efficiency",
                    "N_{vtx}",
                    "Expected #tau efficiency"
)

# DMF plots
NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'dmf',0,[11,0,11],
                    'tau_Ele_effi_dmf', 0, 1.05,
                    "Expected #tau efficiency",
                    "#tau_{h} decay mode",
                    "Expected #tau efficiency"
)



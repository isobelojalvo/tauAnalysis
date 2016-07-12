'''
Usage:python plot.py RootFile.root label[optional]

Script to make some quick fake rate plots to test ntuple generation.


Author: L. Dodd, UW Madison

'''

from subprocess import Popen
from sys import argv, exit, stdout, stderr
import os
import ROOT
import array

# Set overall style of plots
ROOT.gROOT.SetStyle("Plain")
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetTitleBorderSize(0)
#ROOT.gStyle.SetPadTopMargin(0.15)
ROOT.gStyle.SetPadGridY(1) 
ROOT.gStyle.SetTitleX(0.1)
ROOT.gStyle.SetTitleY(0.97)
colors=[ROOT.kCyan,ROOT.kGreen,ROOT.kOrange,ROOT.kRed,ROOT.kBlue]  #Add more colors for >5 sets of points in a single plot
styles=[20,21,22,23,33]

######## File #########
if len(argv) < 2:
   print 'Usage:python plot.py RootFile.root label[optional]'
   exit()

infile = argv[1]
ntuple_file = ROOT.TFile(infile)

######## LABEL & SAVE WHERE #########
if len(argv)>2:
   saveWhere='~/private/output/tauAnalysis/8_0_10/ZtoTT/Fake Rate/'+argv[2]+'_'
else:
   saveWhere='~/private/output/tauAnalysis/8_0_10/ZtoTT/Fake Rate/'

#####################################
#Get fakerate NTUPLE                 #
#####################################
byLooseCmbIso3 = ntuple_file.Get("byLooseCombinedIsolationDeltaBetaCorr3Hits/Gen. Ele")
byMedCmbIso3 = ntuple_file.Get("byMediumCombinedIsolationDeltaBetaCorr3Hits/Gen. Ele")
byTightCmbIso3 = ntuple_file.Get("byTightCombinedIsolationDeltaBetaCorr3Hits/Gen. Ele")
ntrlIsoPtSum = ntuple_file.Get("neutralIsoPtSum/Gen. Ele")
puCorrPtSum = ntuple_file.Get("puCorrPtSum/Gen. Ele")
MuLoose3 = ntuple_file.Get("againstMuonLoose3/Gen. Ele")
MuTight3 = ntuple_file.Get("againstMuonTight3/Gen. Ele")
EleVLooseMVA6 = ntuple_file.Get("againstElectronVLooseMVA6/Gen. Ele")
EleLooseMVA6 = ntuple_file.Get("againstElectronLooseMVA6/Gen. Ele")
EleMediumMVA6 = ntuple_file.Get("againstElectronMediumMVA6/Gen. Ele")
EleTightMVA6 = ntuple_file.Get("againstElectronTightMVA6/Gen. Ele")
EleVTightMVA6 = ntuple_file.Get("againstElectronVTightMVA6/Gen. Ele")
print EleVTightMVA6
canvas = ROOT.TCanvas("asdf", "adsf", 1200, 800)

def make_plot(tree, variable, selection, binning, xaxis='', title=''):
    ''' Plot a variable using draw and return the histogram '''
    draw_string = "%s>>htemp(%s)" % (variable, ", ".join(str(x) for x in binning))
    tree.Draw(draw_string, selection, "goff")
    output_histo = ROOT.gDirectory.Get("htemp").Clone()
    if variable == '':#elePt
	inclusiveBinning = array.array('d', [20,25,30,35,40,45,50,55,60,65,70,80,90,100,120])
	output_histo = output_histo.Rebin(14,"rebinned",inclusiveBinning)
    output_histo.SetTitle("")
    return output_histo

def make_fakerate(denom, num):
    ''' Make a fake rate graph with the style '''
    eff = ROOT.TGraphAsymmErrors(num, denom)
    eff.SetMarkerStyle(20)
    eff.SetMarkerSize(1.5)
    eff.SetLineColor(ROOT.kBlack)
    return eff

def make_num(ntuple, variable,PtCut,binning):
    num = make_plot(
        ntuple, variable,
	"fakeEle",
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

def produce_fakerate(ntuple, variable, PtCut,binning, filename,color,style):
    denom = make_denom(ntuple, variable,PtCut,binning)
    num = make_num(ntuple,variable,PtCut,binning)
    l1 = make_fakerate(denom,num)
    l1.SetMarkerColor(color)
    l1.SetMarkerStyle(style)
    return l1

#Accepts any number of ntuples and plots them for comparison; works well up to at least 5
def NewCompareEfficiencies(ntuples, legends, variable, PtCut, binning, filename, framemin, framemax, title='', xaxis='',yaxis=''):
	frame = ROOT.TH1F("frame", "frame", *binning)
	histlist = []
	for i in range(len(ntuples)):
		histlist.append(produce_fakerate(ntuples[i],variable, PtCut,binning, filename,colors[i],styles[i]))
    	frame.SetMaximum(framemax)
    	frame.SetMinimum(framemin)
    	frame.GetXaxis().SetTitle(xaxis)
    	frame.GetYaxis().SetTitle(yaxis)
    	frame.GetYaxis().SetTitleOffset(1.4)
    	frame.GetYaxis().CenterTitle()
    	frame.GetXaxis().CenterTitle()
	frame.GetYaxis().SetNdivisions(10)
    	frame.UseCurrentStyle()
	frame.Draw()
	frame.SetTitle('CMS #it{Simulation}  High-mass Z/#gamma*#rightarrowLL  CMSSW 8_0_10')
	#frame.SetTitle('CMS #it{Simulation} Z*#rightarrow#tau#tau CMSSW 7_6_5')
	canvas.SetLogy()

	for i in range(len(histlist)):
		if i == 0:
			histlist[i].Draw('pe')
		else:
			histlist[i].Draw('pesame')
		num = i

	if variable == 'dmf':
        	text = ROOT.TPaveText(0.1,1e-4,1,3e-4)
                text.AddText("h^{#pm}")                 
                text.SetLineStyle(0)
                text.SetFillColor(0)
                text2 = ROOT.TPaveText(1.1,5e-5,3,2e-4)
                text2.AddText("h^{#pm}#pi^{0}")
                text2.AddText("h^{#pm}#pi^{0}#pi^{0}")
                text2.SetLineStyle(0)
                text2.SetFillColor(0)
                text3 = ROOT.TPaveText(9,1.5e-4,10.9,7e-4)
                text3.AddText("h^{#pm}h^{#mp}h^{#pm}")
                text3.SetLineStyle(0)
                text3.SetFillColor(0)
                text.Draw()
                text2.Draw()
                text3.Draw()
	legend = ROOT.TLegend(0.5,0.1,0.9,0.1+0.05*len(legends), "", "brNDC")
    	legend.SetFillColor(ROOT.kWhite)
    	legend.SetBorderSize(1)
	for i in range(len(legends)):
		legend.AddEntry(histlist[i],legends[i],'pe')
    	legend.Draw()
    	saveas = saveWhere+filename+'.png'
    	print saveas
    	canvas.SaveAs(saveas)

################################################################################
# Fake Efficiency for a 20 GeV cut on tau Pt 
################################################################################
## pT plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'elePt', 20, [24,20,500],#variable, ptcut, binning
                    'tau_80x_ZtoTT_iso_fakerate_pT',3e-1,1, #filename, lower bound (y), upper bound (y)
                    "Tau Fake Efficiency (Electrons)",#title
                    "Electron p_{T} (GeV)",#xaxis
                    "Expected e #rightarrow #tau fake probability" #yaxis            
)

NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'elePt',20,[24,20,500],
                    'tau_80x_ZtoTT_PtSum_fakerate_pT',1e-1,1,
                    "Tau Fake Efficiency (Electrons)",
                    "Electron p_{T} (GeV)",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'elePt',20,[24,20,500],
                    'tau_80x_ZtoTT_Mu_fakerate_pT',1e-5,1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "Muon p_{T} (GeV)",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'elePt',20,[24,20,500],
                    'tau_80x_ZtoTT_Ele_fakerate_pT',1e-6,1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "Electron p_{T} (GeV)",
                    "Expected  e #rightarrow #tau  fake probability"
)

## eta plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'eleEta', 20, [20,-2.4,2.4],#variable, ptcut, binning
                    'tau_80x_ZtoTT_iso_fakerate_eta', 3e-1, 1,#filename, lower bound (y), upper bound (y)
                    "Tau Fake Efficiency (Electrons)",#title
                    "Muon #Eta",#xaxis
                    "Expected e#rightarrow#tau fake probability" #yaxis             
)
NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'eleEta',20,[20,-2.4,2.4],
                    'tau_80x_ZtoTT_PtSum_fakerate_eta',1e-1, 1,
                    "Tau Fake Efficiency (Electrons)",
                    "Muon #Eta",
                    "Expected e#rightarrow#tau fake probability"
)
NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'eleEta',20,[20,-2.4,2.4],
                    'tau_80x_ZtoTT_Mu_fakerate_eta',1e-5, 1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "Muon #Eta",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'eleEta',20,[20,-2.4,2.4],
                    'tau_80x_ZtoTT_Ele_fakerate_eta',1e-4, 1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "Muon #eta",
                    "Expected e#rightarrow#tau fake probability"
)

## nvtx plots
NewCompareEfficiencies([byLooseCmbIso3,byMedCmbIso3,byTightCmbIso3],['byLooseCombIsoDBCorr3Hits','byMediumCombIsoDBCorr3Hits','byTightCombIsoDBCorr3Hits'],'nvtx', 20, [20,0,35],#variable, ptcut, binning
                    'tau_80x_ZtoTT_iso_fakerate_nvtx',3e-1, 1,#filename, lower bound (y), upper bound (y)
                    "Tau Fake Efficiency (Electrons)",#title
                    "N_{vtx}",#xaxis
                    "Expected e#rightarrow#tau fake probability" #yaxis             
)

NewCompareEfficiencies([ntrlIsoPtSum,puCorrPtSum],['neutralIsoPtSum','puCorrPtSum'],'nvtx',20,[20,0,35],
                    'tau_80x_ZtoTT_PtSum_fakerate_nvtx',1e-1, 1,
                    "Tau Fake Efficiency (Electrons)",
                    "N_{vtx}",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([MuLoose3,MuTight3],['againstMuonLoose3','againstMuonTight3'],'nvtx',20,[20,0,35],
                    'tau_80x_ZtoTT_Mu_fakerate_nvtx',1e-5, 1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "N_{vtx}",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'nvtx',20,[20,0,35],
                    'tau_80x_ZtoTT_Ele_fakerate_nvtx',1e-4, 1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "N_{vtx}",
                    "Expected e#rightarrow#tau fake probability"
)

NewCompareEfficiencies([EleVLooseMVA6,EleLooseMVA6,EleMediumMVA6,EleTightMVA6,EleVTightMVA6],['againstElectronVLooseMVA6','againstElectronLooseMVA6','againstElectronMediumMVA6','againstElectronTightMVA6','againstElectronVTightMVA6'],'dmf',20,[11,0,11],
                    'tau_80x_ZtoTT_Ele_fakerate_dmf',1e-6, 1e-1,
                    "Tau Fake Efficiency (Electrons)",
                    "#tau decay mode",
                    "Expected e#rightarrow#tau fake probability"
)



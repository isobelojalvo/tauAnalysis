import FWCore.ParameterSet.Config as cms
import os


from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
#with open('files_MVA_DY.txt') as f:
#    options.inputFiles = f.readlines()

#input cmsRun options
options.outputFile = "MiniAOD_FR_80x_DYToLL.root"
options.parseArguments()
#name the process
process = cms.Process("TreeProducerFromMiniAOD")

#Make the framework shutup
process.load('FWCore/MessageService/MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 10000;
process.MessageLogger.cerr.threshold = cms.untracked.string('INFO')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag

#50 ns global tag for MC replace with 'GR_P_V56' for prompt reco. https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Prompt_reconstruction_Global_Tag 
#process.GlobalTag = GlobalTag(process.GlobalTag, '76X_mcRun2_asymptotic_RunIIFall15DR76_v1', '')
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_v6', '')

#how many events to run over
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
#input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)

##################################################
# Main
process.againstElectronVLooseMVA6 = cms.EDAnalyzer("MiniAODfakeRate_ZToEE",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    electrons = cms.InputTag("slimmedElectrons"), muons = cms.InputTag("slimmedMuons"),
    tauID = cms.string("againstElectronVLooseMVA6"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.againstElectronLooseMVA6 = cms.EDAnalyzer("MiniAODfakeRate_ZToEE",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    electrons = cms.InputTag("slimmedElectrons"), muons = cms.InputTag("slimmedMuons"),
    tauID = cms.string("againstElectronLooseMVA6"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.againstElectronMediumMVA6 = cms.EDAnalyzer("MiniAODfakeRate_ZToEE",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    electrons = cms.InputTag("slimmedElectrons"), muons = cms.InputTag("slimmedMuons"),
    tauID = cms.string("againstElectronMediumMVA6"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.againstElectronTightMVA6 = cms.EDAnalyzer("MiniAODfakeRate_ZToEE",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    electrons = cms.InputTag("slimmedElectrons"), muons = cms.InputTag("slimmedMuons"),
    tauID = cms.string("againstElectronTightMVA6"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.againstElectronVTightMVA6 = cms.EDAnalyzer("MiniAODfakeRate_ZToEE",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    electrons = cms.InputTag("slimmedElectrons"), muons = cms.InputTag("slimmedMuons"),
    tauID = cms.string("againstElectronVTightMVA6"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)

##################################################
##Global sequence

process.p = cms.Path(
		     process.againstElectronVLooseMVA6*
		     process.againstElectronLooseMVA6*
		     process.againstElectronMediumMVA6*
		     process.againstElectronTightMVA6*
		     process.againstElectronVTightMVA6
                     )

#output file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

#print out all processes used when running- useful check to see if module ran
#UNCOMMENT BELOW
#dump_file = open('dump.py','w')
#dump_file.write(process.dumpPython())

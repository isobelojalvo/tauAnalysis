import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

#input cmsRun options
options = VarParsing ('analysis')
#with open('files_SUSYggH.txt') as f:
#    options.inputFiles = f.readlines()
#options.inputFiles = "file:/afs/hep.wisc.edu/home/ncinko/private/CMSSW_8_0_10/src/RecoTauTag/tauAnalysis/test/A689C644-EF42-E611-B1E4-002590200A68.root"
options.outputFile = "twoprong.root"
options.parseArguments()

#name the process
process = cms.Process("TreeProducerFromMiniAOD")
process.load('FWCore/MessageService/MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 5000;
process.MessageLogger.cerr.threshold = cms.untracked.string('INFO')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#50 ns global tag for MC replace with 'GR_P_V56' for prompt reco. https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Prompt_reconstruction_Global_Tag 
from Configuration.AlCa.GlobalTag import GlobalTag
#Make sure Global Tag mathes input file type
#process.GlobalTag = GlobalTag(process.GlobalTag, '76X_mcRun2_asymptotic_RunIIFall15DR76_v1', '')
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_v6', '')

#how many events to run over
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)


##################################################
# Main
process.byLooseCombinedIsolationDeltaBetaCorr3Hits = cms.EDAnalyzer("MiniAODtwoprong",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),    
    PFCandidates = cms.InputTag("packedPFCandidates"),
    tauID = cms.string("byLooseCombinedIsolationDeltaBetaCorr3Hits"), 
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.byMediumCombinedIsolationDeltaBetaCorr3Hits = cms.EDAnalyzer("MiniAODtwoprong",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    PFCandidates = cms.InputTag("packedPFCandidates"),
    tauID = cms.string("byMediumCombinedIsolationDeltaBetaCorr3Hits"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.byTightCombinedIsolationDeltaBetaCorr3Hits = cms.EDAnalyzer("MiniAODtwoprong",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    PFCandidates = cms.InputTag("packedPFCandidates"),
    tracks = cms.InputTag("tracks"),
    tauID = cms.string("byTightCombinedIsolationDeltaBetaCorr3Hits"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)
process.byNone = cms.EDAnalyzer("MiniAODtwoprong",
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taus = cms.InputTag("slimmedTaus"),
    PFCandidates = cms.InputTag("packedPFCandidates"),
    tracks = cms.InputTag("tracks"),
    tauID = cms.string("decayModeFindingNewDMs"),
    packed = cms.InputTag("packedGenParticles"),
    pruned = cms.InputTag("prunedGenParticles")
)

###################################################
#Global sequence

process.p = cms.Path(
                     process.byLooseCombinedIsolationDeltaBetaCorr3Hits*
             process.byMediumCombinedIsolationDeltaBetaCorr3Hits*
             process.byTightCombinedIsolationDeltaBetaCorr3Hits*
             process.byNone
                     )

process.TFileService = cms.Service("TFileService",
        fileName = cms.string(options.outputFile)
)

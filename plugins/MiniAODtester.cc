// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"

//
// class declaration
//

class MiniAODtester : public edm::EDAnalyzer {
   public:
      explicit MiniAODtester(const edm::ParameterSet&);
      ~MiniAODtester();

   private:
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
      edm::EDGetTokenT<pat::TauCollection> tauToken_;
      edm::EDGetTokenT<pat::JetCollection> jetToken_;

      std::string tauID_;

      TTree* tree;
      Float_t tauPt_;
      Float_t tauEta_;
      Int_t dmf_;
      Int_t tauIndex_;
      Int_t passDiscr_;
      double maxDR_;
};

MiniAODtester::MiniAODtester(const edm::ParameterSet& iConfig):
	vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
	tauToken_(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"))),
	jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jets")))
{

	tauID_    = iConfig.getParameter<std::string>("tauID");

	edm::Service<TFileService> fs;

	tree = fs->make<TTree>("Ntuple", "Ntuple");
	tree->Branch("tauPt", &tauPt_,"tauPt_/F");
	tree->Branch("tauEta", &tauEta_,"tauEta_/F");
	tree->Branch("tauIndex", &tauIndex_,"tauIndex_/I");
	tree->Branch("passDiscr", &passDiscr_,"passDiscr_/I");
	tree->Branch("dmf", &dmf_,"dmf_/I");

}

MiniAODtester::~MiniAODtester()
{
}


	void
MiniAODtester::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	edm::Handle<reco::VertexCollection> vertices;
	iEvent.getByToken(vtxToken_, vertices);
	if (vertices->empty()) return; // skip the event if no PV found
	const reco::Vertex &PV = vertices->front();

	edm::Handle<pat::TauCollection> taus;
	iEvent.getByToken(tauToken_, taus);
        int tau_position=-1;
	for (const pat::Tau &tau : *taus) {
		tau_position++;
		if (tau.pt() < 20 ) continue;
		tauPt_=tau.pt();
		tauEta_=tau.eta();
		dmf_=tau.tauID("decayModeFinding");//dmf always==1 in miniaod
		tauIndex_=tau_position;
		if (tau.tauID(tauID_)) passDiscr_ = 1;
		else passDiscr_=0;
//		printf("tau  with pt %4.1f, dxy signif %.1f, ID(byMediumCombinedIsolationDeltaBetaCorr3Hits) %.1f, lead candidate pt %.1f, pdgId %d \n",
//				tau.pt(), tau.dxy_Sig(), tau.tauID("byMediumCombinedIsolationDeltaBetaCorr3Hits"), tau.leadCand()->pt(), tau.leadCand()->pdgId());
		tree->Fill();//produce flat ntuple
	}


}

//define this as a plug-in
DEFINE_FWK_MODULE(MiniAODtester);

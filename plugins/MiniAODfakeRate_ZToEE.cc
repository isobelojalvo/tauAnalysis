// system include files
#include <memory>
#include <vector>
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
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "helpers.h"
#include <iostream>
#include "TMath.h"
#include <algorithm>
//
// class declaration
//
class MiniAODfakeRate_ZToEE : public edm::EDAnalyzer {
	public:
		explicit MiniAODfakeRate_ZToEE(const edm::ParameterSet&);
		~MiniAODfakeRate_ZToEE();

	private:
		virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

		// ----------member data ---------------------------
		edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
		edm::EDGetTokenT<pat::TauCollection> tauToken_;
                edm::EDGetTokenT<std::vector <reco::GenParticle> > prunedGenToken_;
                edm::EDGetTokenT<std::vector < reco::GenParticle> >packedGenToken_;
		edm::EDGetTokenT<pat::ElectronCollection> electronToken_;
		edm::EDGetTokenT<pat::MuonCollection> muonToken_;
		std::string tauID_;

		TTree* tree;
		TTree* tree2;
		Int_t tauIndex_=0;
		Float_t tauPt_=-999;
		Float_t tauEta_=-999;
		Float_t elePt_=-999;
		Float_t eleEta_=-999;
		Float_t muPt_=-999;
		Float_t muEta_=-999;
		Int_t nvtx_=-999;
		Int_t fakeEle_=0;
		Int_t fakeMu_=0;
		Int_t dmf_ = -1;
		double maxDR_;
};

MiniAODfakeRate_ZToEE::MiniAODfakeRate_ZToEE(const edm::ParameterSet& iConfig):
	vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
	tauToken_(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"))),
	prunedGenToken_(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("pruned"))),
	packedGenToken_(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("packed"))),
	electronToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("electrons"))),
	muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons")))

{
	tauID_    = iConfig.getParameter<std::string>("tauID");
	edm::Service<TFileService> fs;
	
	tree = fs->make<TTree>("Gen. Ele", "Gen. Ele");  //Tree for generated electrons
	tree->Branch("elePt", &elePt_,"elePt_/F");
	tree->Branch("eleEta", &eleEta_,"eleEta_/F");
	tree->Branch("tauEta", &tauEta_,"tauEta_/F");
	tree->Branch("tauIndex",&tauIndex_,"tauIndex/I");
	tree->Branch("nvtx",&nvtx_,"nvtx/I");
	tree->Branch("fakeEle",&fakeEle_,"fakeEle/I");
	tree->Branch("dmf",&dmf_,"dmf/I");

//Uncomment tree to look at mu->tau fake rate
	
//	tree2 = fs->make<TTree>("Gen. Mu","Gen. Mu"); //Tree for generated muons
//	tree2->Branch("muPt", &elePt_,"muPt_/F");
//	tree2->Branch("muEta", &eleEta_,"muEta_/F");
//	tree2->Branch("tauEta", &tauEta_,"tauEta_/F");
//	tree2->Branch("tauIndex",&tauIndex_,"tauIndex/I");
//	tree2->Branch("nvtx",&nvtx_,"nvtx/I");
//	tree2->Branch("fakeMu",&fakeMu_,"fakeMu/I");

	
}

MiniAODfakeRate_ZToEE::~MiniAODfakeRate_ZToEE()
{
}

	void
MiniAODfakeRate_ZToEE::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	edm::Handle<reco::VertexCollection> vertices;
	iEvent.getByToken(vtxToken_, vertices);
	if (vertices->empty()) return; // skip the event if no PV found
	//const reco::Vertex &PV = vertices->front();

	edm::Handle<pat::TauCollection> taus;
	iEvent.getByToken(tauToken_, taus);

	edm::Handle<std::vector<reco::GenParticle>> genParticles;
	iEvent.getByToken(prunedGenToken_, genParticles);

	std::vector<const reco::GenParticle*> GenTaus;
	std::vector<const reco::GenParticle*> GenEles;
	std::vector<const reco::GenParticle*> GenMus;

	for(std::vector<reco::GenParticle>::const_iterator genParticle = genParticles->begin(); genParticle != genParticles->end(); genParticle++ ){
	  if(TMath::Abs(genParticle->pdgId()) == 15) GenTaus.push_back(&(*genParticle));
	  if(TMath::Abs(genParticle->pdgId()) == 11) GenEles.push_back(&(*genParticle));
	  if(TMath::Abs(genParticle->pdgId()) == 13) GenMus.push_back(&(*genParticle));
	}

        if (GenTaus.size()>0 || GenMus.size()>0) return; // we want a DY -> ee sample 
	nvtx_=vertices->size();

	for (int i = 0;(unsigned)i<GenEles.size(); i++) {  //looping through only generated electrons
		elePt_ = GenEles[i]->pt();
		eleEta_ = GenEles[i]->eta(); 
		if (elePt_ > 20 && TMath::Abs(eleEta_) < 2.3){  
			tauIndex_ = 0;
			for (const pat::Tau &tau : *taus) {
				tauPt_ = tau.pt();
				tauEta_ = tau.eta();
				fakeEle_ = 0;//assume the tau is not faked by an electron
				bool fake_discr = tau.tauID(tauID_)>.5 && tau.tauID("byLooseCombinedIsolationDeltaBetaCorr3Hits"); //does tau fake additional discriminator?
				if (tau.tauID("decayModeFinding")>0.5 && tau.pt() > 20 && TMath::Abs(tau.eta())<2.3 && fake_discr && deltaR(*GenEles[i],tau)<0.3) { // if the tau passes the critera				   
					dmf_ = tau.decayMode();
					fakeEle_ = 1; //if tau passes set the electron output to "fake". e.g. this electron faked a tau
					break;	//end tau matching search once we know a tau was faked by an electron
				}
				tauIndex_++;//which number tau was faked (good to know if highest pt or second highest pt tau, etc.)
			}//end matching tau to ele
			tree->Fill(); //fill gen. electron tree 
		}
	}

//Uncomment loop below to look at mu->tau fake rate

//        for (size_t i = 0; i < GenMus.size(); i++) {
//		//output all muons to denominator
//		muPt_ = GenMus[i]->pt();
//		muEta_ = GenMus[i]->eta();
//		if (muPt_ > 20 and abs(muEta_) < 2.3){  //slimmed muon collection- should we add passes loose discrimination? 
//			tauIndex_ = 0;
//			for (const pat::Tau &tau : *taus) {
//				tauPt_ = tau.pt();
//				tauEta_ = tau.eta();
//				fakeMu_ = 0;//assume the tau is not faked by a muon
//				bool fake_discr = tau.tauID(tauID_)>.5; //does tau fake additional discriminator?
//				if (tau.tauID("decayModeFinding")>0.5 && tau.pt() > 20 && abs(tau.eta())<2.3 && fake_discr && deltaR(*GenMus[i],tau)<0.3) { // if the tau passes the critera
//					fakeMu_ = 1; //if tau passes set the muon output to "fake". e.g. this muon faked a tau
//					break;	//end tau matching search once we know a tau was faked by a muon
//				}
//				tauIndex_++;//which number tau was faked (good to know if highest pt or second highest pt tau, etc.)
//			}//end matching tau to mu
//			tree2->Fill(); //fill gen. muon tree 
//		}
//	}
 

} // end analyze
//define this as a plug-in
DEFINE_FWK_MODULE(MiniAODfakeRate_ZToEE);

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
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"
#include "helpers.h"
#include "DataFormats/Math/interface/deltaR.h"
#include <iostream>
#include "TMath.h"
#include "TLorentzVector.h"

// function declarations

// class declaration
class MiniAODeffi_dmf : public edm::EDAnalyzer {
	public:
		explicit MiniAODeffi_dmf(const edm::ParameterSet&);
		~MiniAODeffi_dmf();

	private:
		virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

		// ----------member data ---------------------------
		edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
		edm::EDGetTokenT<pat::TauCollection> tauToken_;
		std::string tauID_;
		edm::EDGetTokenT<std::vector <reco::GenParticle> > prunedGenToken_;
                edm::EDGetTokenT<std::vector < reco::GenParticle> >packedGenToken_;

		TTree* tree;
		Float_t tauPt_;
		Float_t tauEta_;
    	Float_t recoTauPt_;
		Float_t recoTauEta_;
		Float_t tauMass_;
		Int_t tauIndex_;
		Int_t nvtx_;
		Int_t goodReco_;
		Int_t decayMode_;
        Int_t recoDecayMode_;
        Float_t recoTauMass_;
		double maxDR_;
		bool good_dz_;
		bool good_dr_;
};

MiniAODeffi_dmf::MiniAODeffi_dmf(const edm::ParameterSet& iConfig):
	vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
	tauToken_(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"))),
	prunedGenToken_(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("pruned"))),
	packedGenToken_(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("packed")))
{
	tauID_    = iConfig.getParameter<std::string>("tauID");
	edm::Service<TFileService> fs;

	tree = fs->make<TTree>("Ntuple", "Ntuple");
	tree->Branch("tauPt", &tauPt_,"tauPt_/F");
	tree->Branch("tauEta", &tauEta_,"tauEta_/F");
	tree->Branch("recoTauPt", &recoTauPt_,"recoTauPt_/F");
	tree->Branch("recoTauEta", &recoTauEta_,"recoTauEta_/F");
	tree->Branch("tauIndex", &tauIndex_,"tauIndex_/I");
	tree->Branch("nvtx",&nvtx_,"nvtx_/I");
	tree->Branch("goodReco",&goodReco_,"goodReco_/I");
	tree->Branch("tauMass",&tauMass_,"tauMass_/F");
    tree->Branch("recoTauMass",&recoTauMass_,"recoTauMass_/F");
	tree->Branch("decayMode",&decayMode_,"decayMode_/I");
    tree->Branch("recoDecayMode",&recoDecayMode_,"recoDecayMode_/I");
}

MiniAODeffi_dmf::~MiniAODeffi_dmf()
{
}

	void

MiniAODeffi_dmf::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{	
	edm::Handle<reco::VertexCollection> vertices;
	iEvent.getByToken(vtxToken_, vertices);
	nvtx_=vertices->size();
//	const reco::Vertex &PV = vertices->front();
	edm::Handle<pat::TauCollection> taus;
	iEvent.getByToken(tauToken_, taus);
 	edm::Handle<std::vector<reco::GenParticle> > genParticles;
 	iEvent.getByToken(packedGenToken_, genParticles);

 	std::vector<const reco::GenParticle*> GenTaus;
 	std::vector<const reco::GenParticle*> GenEles;
 	std::vector<const reco::GenParticle*> GenMus;
 	//Place generated leptons into separate lists
	for(std::vector<reco::GenParticle>::const_iterator genParticle = genParticles->begin(); genParticle != genParticles->end(); genParticle++){
		if(TMath::Abs(genParticle->pdgId()) == 15) GenTaus.push_back(&(*genParticle));
		if(TMath::Abs(genParticle->pdgId()) == 11) GenEles.push_back(&(*genParticle));
		if(TMath::Abs(genParticle->pdgId()) == 13) GenMus.push_back(&(*genParticle));
	}
		
	goodReco_ = 0;
	for (size_t i=0;i<GenTaus.size();i++){
	//	std::cout << "\n New gen. tau";

        std::vector<const reco::GenParticle*> genTauDaughters;
        findDaughters(GenTaus[i], genTauDaughters);
        decayMode_ = GetDecayMode(genTauDaughters);
        recoDecayMode_ = -1;
        recoTauMass_ = -1;
        recoTauPt_ = -1;
        recoTauEta_ = -4;
	//	std::cout << " Decay Mode: " << decayMode_;
		goodReco_ = 0;	//Assume gen. tau is not reconstructed
		reco::Candidate::LorentzVector p4_vis = GetVisibleP4(genTauDaughters);	//only look at visible decay products of gen. tau
		tauMass_ = p4_vis.mass();
		tauEta_ = p4_vis.eta();
		tauPt_ = p4_vis.pt();
		if(p4_vis.pt() > 20 && TMath::Abs(p4_vis.eta()) < 2.3 && isHadronic(GenTaus[i])){ //isHadronic(GenTaus[i])
			tauIndex_ = 0;	
			for(const pat::Tau &tau : *taus){
				if(tau.pt()>20 && TMath::Abs(tau.eta())<2.3 && tau.tauID("decayModeFindingNewDMs")>0.5 && reco::deltaR(tau.eta(),tau.phi(),p4_vis.eta(),p4_vis.phi())<0.3 &&tau.tauID(tauID_)>0.5){
					goodReco_ = 1;	//reco. tau goes in efficiency numerator
                    recoDecayMode_= tau.decayMode();
                    recoTauMass_= tau.mass();
                    recoTauPt_ = tau.pt();
                    recoTauEta_ = tau.eta();
					break;	//end tau loop once we know there is a good reconstruction
				}
				++tauIndex_;
			}
			if (goodReco_==0) tauIndex_=-1;		
			tree->Fill();	//gen. tau goes in efficiency denominator
		}
	}
}
//define this as a plug-in
DEFINE_FWK_MODULE(MiniAODeffi_dmf);

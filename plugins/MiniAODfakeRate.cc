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
#include "DataFormats/Math/interface/deltaR.h"
#include "helpers.h"
#include <iostream>
#include <algorithm>
//
// class declaration
//

class MiniAODfakeRate_alt : public edm::EDAnalyzer {
	public:
		explicit MiniAODfakeRate_alt(const edm::ParameterSet&);
		~MiniAODfakeRate_alt();

	private:
		virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

		// ----------member data ---------------------------
		edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
		edm::EDGetTokenT<pat::TauCollection> tauToken_;
		edm::EDGetTokenT<pat::JetCollection> jetToken_;
                edm::EDGetTokenT<std::vector <reco::GenParticle> > prunedGenToken_;
                edm::EDGetTokenT<std::vector < pat::PackedGenParticle> >packedGenToken_;

		std::string tauID_;

		TTree* tree;
		Float_t jetPt_;
		Float_t jetEta_;
		Int_t jetIDLoose_;
		Int_t jetIDMed_;
		Int_t jetIDTight_;
		Int_t genMatchedTau_;
		Int_t isFake_;
		Int_t tauIndex_;
		Int_t dmf_;
		Int_t passDiscr_;
		Float_t tauPt_;
		Float_t tauEta_;
		Float_t jetRefEta_;
		Float_t jetRefPt_;
		Int_t nvtx_;

		double maxDR_;
};

MiniAODfakeRate_alt::MiniAODfakeRate_alt(const edm::ParameterSet& iConfig):
	vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
	tauToken_(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"))),
	jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jets"))),
	prunedGenToken_(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("pruned"))),
	packedGenToken_(consumes<std::vector<pat::PackedGenParticle> >(iConfig.getParameter<edm::InputTag>("packed")))
{

	tauID_    = iConfig.getParameter<std::string>("tauID");

	edm::Service<TFileService> fs;

	tree = fs->make<TTree>("Ntuple", "Ntuple");
	tree->Branch("jetPt", &jetPt_,"jetPt_/F");
	tree->Branch("tauPt", &tauPt_,"tauPt_/F");
	tree->Branch("jetEta", &jetEta_,"jetEta_/F");
	tree->Branch("tauEta", &tauEta_,"tauEta_/F");
	tree->Branch("jetRefPt",&jetRefPt_,"jetRefPt/F");
	tree->Branch("jetRefEta",&jetRefEta_,"jetRefEta/F");
	tree->Branch("jetIDLoose",&jetIDLoose_,"jetIDLoose/I");
	tree->Branch("jetIDMed",&jetIDMed_,"jetIDMed/I");
	tree->Branch("jetIDTight",&jetIDTight_,"jetIDTight/I");
	tree->Branch("genMatchedTau",&genMatchedTau_,"genMatchedTau/I");
	tree->Branch("dmf",&dmf_,"dmf/I");
	tree->Branch("isFake",&isFake_,"isFake/I");
	tree->Branch("tauIndex",&tauIndex_,"tauIndex/I");
	tree->Branch("passDiscr",&passDiscr_,"passDiscr/I");
	tree->Branch("nvtx",&nvtx_,"nvtx/I");
	maxDR_ = 0.4;
}

MiniAODfakeRate_alt::~MiniAODfakeRate_alt()
{
}

	void
MiniAODfakeRate_alt::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	edm::Handle<reco::VertexCollection> vertices;
	iEvent.getByToken(vtxToken_, vertices);
	if (vertices->empty()) return; // skip the event if no PV found
	const reco::Vertex &PV = vertices->front();

	edm::Handle<pat::TauCollection> taus;
	iEvent.getByToken(tauToken_, taus);
	edm::Handle<pat::JetCollection> jets;
	iEvent.getByToken(jetToken_, jets);

	edm::Handle<std::vector<reco::GenParticle> > genParticles;
	iEvent.getByToken(prunedGenToken_, genParticles);

	std::vector<const reco::GenParticle*> GenTaus;
	std::vector<const reco::GenParticle*> GenEles;
	std::vector<const reco::GenParticle*> GenMus;
	//add code to make this into GenTaus/GenEles/GenMus
	for(std::vector<reco::GenParticle>::const_iterator genParticle = genParticles->begin(); genParticle != genParticles->end(); genParticle++){
		if(TMath::Abs(genParticle->pdgId()) == 15) GenTaus.push_back(&(*genParticle));
		if(TMath::Abs(genParticle->pdgId()) == 11) GenEles.push_back(&(*genParticle));
		if(TMath::Abs(genParticle->pdgId()) == 13) GenMus.push_back(&(*genParticle));
	}

	int tau_position=-1;
	genMatchedTau_=0;
	isFake_=0;
	dmf_=0;
	passDiscr_=0;
	tauPt_=-999;
	tauEta_=-999;
	tauIndex_=-1;
	jetRefPt_=-999;
	jetPt_=-999;
	jetEta_=-999;
	jetRefEta_=-999;
	jetIDLoose_=0;
	jetIDMed_=0;
	jetIDTight_=0;

	std::vector <int> tau_position_vec;
	std::vector <const pat::Jet*> jet_denom_vec;
	std::vector <const reco::GenParticle*> used_GenTau_vec;

	int iJet = -1;
	jetIDLoose_=0;
        jetIDMed_=0;
        jetIDTight_=0;
        tau_position=-1;
        genMatchedTau_=0;
        isFake_=0;
        dmf_=0;
        passDiscr_=0;
        tauPt_=-999;
        tauEta_=-999;
        tauIndex_=-1;
        jetRefPt_=-999;
        jetPt_=-999;
        jetEta_=-999;
        jetRefEta_=-999;
        nvtx_=vertices->size();
	for (const pat::Jet &jet : *jets) {
		jetPt_=-999;
                jetEta_=-999;
                jetIDLoose_=0;
                jetIDMed_=0;
                jetIDTight_=0;
                iJet++;
		//	pass_vert = (jet.vertex().z() - PV.z()) < .2 && tau.dxy() < .045
		if (jet.pt() > 20&&abs(jet.eta())<2.3&&(isLooseJet(jet))) {
                //std::cout << "The Jet PT is >20, jetEta<2.3, it is loose \n";
        	        jetPt_=jet.pt();
                	jetEta_=jet.eta();
                	jetIDLoose_=isLooseJet(jet);
                	jetIDMed_=isMediumJet(jet);
                	jetIDTight_=isTightJet(jet);
			jet_denom_vec.push_back(&jet);
			tree->Fill();
		} // end if the jet passes the criteria
	} // end denominator for
        jetIDLoose_=0;
        jetIDMed_=0;
        jetIDTight_=0;
        tau_position=-1;
        genMatchedTau_=0;
        isFake_=0;
        dmf_=0;
        passDiscr_=0;
        tauPt_=-999;
        tauEta_=-999;
        tauIndex_=-1;
        jetRefPt_=-999;
        jetPt_=-999;
        jetEta_=-999;
        jetRefEta_=-999;
	//std::cout << "Analyzing a new event\n";
        for (const pat::Tau &tau : *taus) {
                tau_position++;
		tauPt_=tau.pt();
		tauEta_=tau.eta();
		jetPt_=-999;
		jetEta_=-999;
		jetIDLoose_=0;
		jetIDMed_=0;
		jetIDTight_=0;
		isFake_=0;
		genMatchedTau_=0;
		if (vertices->empty()) continue; // skip the tau if no PV found
		//std::cout << "Analyzing Tau number " << tau_position << "\n";
		bool pass_discr = tau.tauID(tauID_)>.5 && tau.tauID("againstElectronVLooseMVA6")>.5 && tau.tauID("againstMuonTight3");
		bool pass_vert = (tau.vertex().z() - PV.z()) < .2; // && tau.dxy() < .045
                if (tau.pt() > 20&&abs(tau.eta())<2.3&& pass_discr && pass_vert) { // if the tau passes the critera
			passDiscr_=1;
			const reco::GenParticle* bestGenTau = findBestGenMatch(tau,GenTaus,maxDR_);
			const reco::GenParticle* bestGenEle = findBestGenMatch(tau,GenEles,maxDR_);
			const reco::GenParticle* bestGenMu = findBestGenMatch(tau,GenMus,maxDR_);
			const pat::Jet *jet = findBestJetMatch(tau,jet_denom_vec,maxDR_);
			//std::cout << "The best Gen Object is " << bestGenTau << "\n";
			//std::cout << "The size of Gen Obj is " << GenTaus.size() << "\n";
			if (bestGenTau != NULL) {
				GenTaus.erase(std::remove(GenTaus.begin(), GenTaus.end(), bestGenTau), GenTaus.end());
				continue;
			}
                        else if (bestGenEle != NULL) {
                                GenEles.erase(std::remove(GenEles.begin(), GenEles.end(), bestGenEle), GenEles.end());
				continue;
                        } // end if tau is near a gen electron
                        else if (bestGenMu != NULL) {
                                GenMus.erase(std::remove(GenMus.begin(), GenMus.end(), bestGenMu), GenMus.end());
				continue;
			} // end if tau is near a gen object
			else if (jet != NULL) {
				isFake_=1;
				jetPt_=jet->pt();
				jetEta_=jet->eta();
				//jetIDLoose_=isLooseJet(*jet);
				//jetIDMed_=isMediumJet(*jet);
				//jetIDTight_=isTightJet(*jet);
				tree->Fill();
				jet_denom_vec.erase(std::remove(jet_denom_vec.begin(), jet_denom_vec.end(), jet), jet_denom_vec.end());
			} // end if tau is near a jet
                }// end if tau meets pt, eta, discriminator cutoffs
        } // end numerator for loop
} // end analyze
//define this as a plug-in
DEFINE_FWK_MODULE(MiniAODfakeRate_alt);

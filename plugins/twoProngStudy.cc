// -*- C++ -*-
//
// Package:    RecoTauTag/twoProngStudy
// Class:      twoProngStudy
// 
/**\class twoProngStudy twoProngStudy.cc RecoTauTag/twoProngStudy/plugins/twoProngStudy.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Provenance/interface/EventAuxiliary.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/TauReco/interface/PFTau.h"
#include "DataFormats/TauReco/interface/PFTauDiscriminator.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "TTree.h"
#include "helpers.h"
#include "TLorentzVector.h" 

#include "FWCore/Framework/interface/EventSetup.h"
//
// class declaration
//

struct genVisTau_data{
  reco::Candidate::LorentzVector p4;
  Float_t genTauPt;
  Int_t numDaughters;
  Int_t charge;
};

class twoProngStudy : public edm::EDAnalyzer {
	public:
		explicit twoProngStudy(const edm::ParameterSet&);
		~twoProngStudy();

		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
                bool checkIfTrackMatchesPF(reco::PFTauRef tauCandidate, reco::TrackRef track);
                void findDaughters(const reco::GenParticle*, std::vector<const reco::GenParticle*>&, int = -1);
                reco::Candidate::LorentzVector getVisMomentum(const reco::GenParticle*, std::vector<const reco::GenParticle*>);
                reco::Candidate::LorentzVector getVisMomentum(const std::vector<const reco::GenParticle*>&, int = 1);
                bool isNeutrino(const reco::GenParticle*);
                bool foundGenTau(reco::PFTauRef tauCandidate, std::vector<const reco::GenParticle*> GenObjects, reco::GenParticle& matchedGenParticle);
  void findTrackByCharge( const std::vector<reco::TrackRef> tracks, reco::PFTauRef tauCandidate, float &candPt, float &candMass, int & candCharge);
                void createVisTaus(std::vector<const reco::GenParticle*> GenObjects, std::vector <genVisTau_data>& genVisTaus);
                bool foundGenTauVis(reco::PFTauRef tauCandidate,  std::vector<genVisTau_data> genParticles, genVisTau_data &matchedGenParticle);
	private:
		virtual void beginJob() override;
		virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
		virtual void endJob() override;

		edm::InputTag tauSrc_;
		edm::InputTag jetSrc_;
		edm::InputTag trackSrc_;
		edm::InputTag discriminatorSrc_;
		TTree* tree;
		Float_t jetPt_;
		Float_t jetEta_;
		Int_t jetIDLoose_;
		Int_t jetIDMed_;
		Int_t jetIDTight_;
		Int_t genMatchedTau_;
		Int_t tauIndex_;
		Float_t dmf_;
		Int_t passDiscr_;
		Float_t tauPt_;
		Float_t tauMass_;
		Int_t tauCharge_;
		Int_t genCharge_;
		Float_t tauEta_;
		Float_t jetRefEta_;
		Float_t jetRefPt_;
                Float_t genVisPt_;
                Float_t genVisEta_;
                Float_t genVisPhi_; 
                Int_t nTracks005;  
  		Int_t nTracks01;
  		Int_t nTracks02;
  		Int_t nTracks03;
  		Int_t nTracks005NM;  
  		Int_t nTracks01NM;
  		Int_t nTracks02NM;
  		Int_t nTracks03NM;
		Int_t nvtx_;
  		Float_t candPt1;  
  		Float_t candMass1;
  		Float_t candPt2;  
  		Float_t candMass2;
  		Float_t candPt3;  
  		Float_t candMass3;
  		Int_t candCharge1;  
  		Int_t candCharge2;  
  		Int_t candCharge3;  
  	       	double maxDR_;
};

twoProngStudy::twoProngStudy(const edm::ParameterSet& cfg)
{
	//now do what ever initialization is needed
	tauSrc_              = cfg.getParameter<edm::InputTag>("recoTau");
	jetSrc_              = cfg.getParameter<edm::InputTag>("recoJet");
	trackSrc_            = cfg.getParameter<edm::InputTag>("generalTracks");
	discriminatorSrc_    = cfg.getParameter<edm::InputTag>("recoTauDiscriminator");

	edm::Service<TFileService> fs;
	//ntuple additions
	tree = fs->make<TTree>("Ntuple", "Ntuple");
	tree->Branch("jetPt", &jetPt_,"jetPt/F");
	tree->Branch("tauPt", &tauPt_,"tauPt/F");
	tree->Branch("tauCharge",&tauCharge_,"tauCharge/I");
	tree->Branch("tauMass", &tauMass_,"tauMass/F");
	tree->Branch("genVisPt", &genVisPt_,"genVisPt/F");
	tree->Branch("jetEta", &jetEta_,"jetEta/F");
	tree->Branch("tauEta", &tauEta_,"tauEta/F");
	tree->Branch("jetRefPt",&jetRefPt_,"jetRefPt/F");
	tree->Branch("jetRefEta",&jetRefEta_,"jetRefEta/F");
	tree->Branch("jetIDLoose",&jetIDLoose_,"jetIDLoose/I");
	tree->Branch("jetIDMed",&jetIDMed_,"jetIDMed/I");
	tree->Branch("jetIDTight",&jetIDTight_,"jetIDTight/I");
	tree->Branch("genCharge",&genCharge_,"genCharge/I");
	tree->Branch("dmf",&dmf_,"dmf/F");
	tree->Branch("tauIndex",&tauIndex_,"tauIndex/I");
	tree->Branch("passDiscr",&passDiscr_,"passDiscr/I");
	tree->Branch("nvtx",&nvtx_,"nvtx/I");
	tree->Branch("nTracks005",&nTracks005,"nTracks005/I");
	tree->Branch("nTracks01",&nTracks01,"nTracks01/I");
	tree->Branch("nTracks02",&nTracks02,"nTracks02/I");
	tree->Branch("nTracks03",&nTracks03,"nTracks03/I");
	tree->Branch("nTracks005NM",&nTracks005NM,"nTracks005NM/I");
	tree->Branch("nTracks01NM",&nTracks01NM,"nTracks01NM/I");
	tree->Branch("nTracks02NM",&nTracks02NM,"nTracks02NM/I");
	tree->Branch("nTracks03NM",&nTracks03NM,"nTracks03NM/I");
	tree->Branch("candPt1",&candPt1,"candPt1/F");
	tree->Branch("candPt2",&candPt2,"candPt2/F");
	tree->Branch("candPt3",&candPt3,"candPt3/F");
	tree->Branch("candMass1",&candMass1,"candMass1/F");
	tree->Branch("candMass2",&candMass2,"candMass2/F");
	tree->Branch("candMass3",&candMass3,"candMass3/F");
	tree->Branch("candCharge1",&candCharge1,"candCharge1/I");
	tree->Branch("candCharge2",&candCharge2,"candCharge2/I");
	tree->Branch("candCharge3",&candCharge3,"candCharge3/I");
	maxDR_ = 0.2;
}


twoProngStudy::~twoProngStudy()
{
	// do anything here that needs to be done at desctruction time
	// (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// ------------ method called for each event  ------------
	void
twoProngStudy::analyze(const edm::Event& evt, const edm::EventSetup& iSetup)
{
	using namespace edm;

	Handle<reco::VertexCollection> vertices;
	evt.getByLabel("offlinePrimaryVertices", vertices);
	if(vertices->size()>0)
	  nvtx_=vertices->size();	
	else return;	

	Handle<reco::PFTauCollection> tauObjects;
	evt.getByLabel(tauSrc_, tauObjects);

	Handle<reco::PFJetCollection> jetObjects;
	evt.getByLabel(jetSrc_, jetObjects);

	Handle<vector<reco::Track> >tracks;
	evt.getByLabel(trackSrc_, tracks);

	Handle<reco::PFTauDiscriminator> discriminator;
	evt.getByLabel(discriminatorSrc_, discriminator);

	Handle<reco::PFTauDiscriminator> DMF; 
	evt.getByLabel("hpsPFTauDiscriminationByDecayModeFinding",DMF);

	int tau_position=-1; genMatchedTau_=0; passDiscr_=0; 
	tauPt_=-20; tauEta_=-20; tauMass_= -20; tauIndex_=-1; 
	jetRefPt_=-20; jetPt_=-20; jetEta_=-20; jetRefEta_=-20; jetIDLoose_=0; jetIDMed_=0;	jetIDTight_=0;
	dmf_=0; 

	std::vector<const reco::GenParticle*> GenObjects = getGenParticleCollection(evt);
	std::vector <int> tau_position_vec;
	std::vector <const reco::PFJet*> jet_denom_vec;
	std::vector <const reco::GenParticle*> used_GenTau_vec;
	std::vector <genVisTau_data> genVisTaus;

	//Create Vis Taus
	createVisTaus(GenObjects,genVisTaus);
	//std::cout<<"nGenVistaus "<<genVisTaus.size()<<std::endl;
	for (unsigned int iTau = 0; iTau<tauObjects->size() ; ++iTau){
	  reco::PFTauRef tauCandidate( tauObjects, iTau);
	  tauCharge_ = tauCandidate->charge();
	  //zero everything
	  genMatchedTau_ = -1;
	  candPt1   = -1;
	  candMass1 = -1;
	  candPt2   = -1;
	  candMass2 = -1;
	  candPt3   = -1;
	  candMass3 = -1;
	  genVisPt_ = -1;
	  genCharge_  = -10;
	  candCharge1 = -10;
	  candCharge2 = -10;
	  candCharge3 = -10;
	  tau_position++;

	  genVisTau_data genTauVis;//(0,0,0,0);
	  if(foundGenTauVis(tauCandidate, genVisTaus, genTauVis)){
	    //set genVis and gen quantities
	    genMatchedTau_=1;
	    genVisPt_ = genTauVis.p4.pt();
	    genCharge_ = genTauVis.charge;
	    //std::cout<< "genTauVisPt: "<<genTauVis.pt()<<" recoTauPt "<< tauCandidate->pt()<<std::endl;
	  }

	  dmf_        = tauCandidate->decayMode();//(*DMF)[tauCandidate];
	  if(dmf_==10)
	    std::cout<< "Three Prong tau -------------- genTauPt: " <<genTauVis.genTauPt<<" genTauVisPt: "<<genTauVis.p4.pt()<<" recoTauPt "<< tauCandidate->pt()<<" recoTauMass "<< tauCandidate->mass()<<" numDaugthers: "<< genTauVis.numDaughters <<std::endl;
	  //std::cout<<"Three Prong tau --------------"<<std::endl;
	  tauPt_      = tauCandidate->pt();
	  tauEta_     = tauCandidate->eta();
	  tauMass_    = tauCandidate->mass();
	  tauCharge_  = tauCandidate->charge();

	  if( (*discriminator)[tauCandidate] > 0.5 ){
	    passDiscr_=1;
	    reco::PFJetRef jet = tauCandidate->jetRef();
	    jetRefPt_  = jet->pt();
	    jetRefEta_ = jet->eta();
	    tauIndex_  = tau_position;
	  }

	  //at least multiple tracks
	  if(dmf_ == 5 || dmf_ == 6 ){
	    nTracks005 = 0;
	    nTracks01  = 0;
	    nTracks02  = 0;
	    nTracks03  = 0;
	    //for(const reco::Track &track : *tracks){
	    for(int t = 0; (unsigned)t < tracks->size(); t++){
	      reco::TrackRef trackRef(tracks,t);
	      double deltaR = reco::deltaR(*tauCandidate,tracks->at(t));
	      if(deltaR<0.05)
		nTracks005++;
	      if(deltaR<0.1)
		nTracks01++;
	      if(deltaR<0.2)
		nTracks02++;
	      if(deltaR<0.3)
		nTracks03++;
	    }
	    //now find the tracks
	    nTracks005NM = 0;
	    nTracks01NM  = 0;
	    nTracks02NM  = 0;
	    nTracks03NM  = 0;

	    std::vector<reco::TrackRef> tracks005NM;
	    std::vector<reco::TrackRef> tracks01NM;
	    std::vector<reco::TrackRef> tracks02NM;
	    for(int t = 0; (unsigned)t < tracks->size(); t++){
	      reco::TrackRef trackRef(tracks,t);
	      double deltaR = reco::deltaR(*tauCandidate,tracks->at(t));
	      bool match = checkIfTrackMatchesPF(tauCandidate,trackRef);
	      if(deltaR<0.05&&!match){
		nTracks005NM++;
		tracks005NM.push_back(trackRef);
	      }
	      if(deltaR<0.1&&!match){
		nTracks01NM++;
		tracks01NM.push_back(trackRef);
	      }
	      if(deltaR<0.2&&!match){
		nTracks02NM++;
		tracks02NM.push_back(trackRef);
	      }
	      if(deltaR<0.3&&!match){
		nTracks03NM++;
	      }
	    }
	    //if(genTauVis.p4.pt()>15)
	    //std::cout<< "genTauPt: " <<genTauVis.genTauPt<<" genTauVisPt: "<<genTauVis.p4.pt()<<" recoTauPt "<< tauCandidate->pt()<<" recoTauMass "<< tauCandidate->mass()<<std::endl;
	    //std::cout<<"genTauVisPt: "<<genTauVis.p4.pt()<<" recoTauPt "<< tauCandidate->pt()<<" recoTauMass "<< tauCandidate->mass()<<std::endl;
	    findTrackByCharge(tracks005NM, tauCandidate, candPt1, candMass1, candCharge1);
	    findTrackByCharge(tracks01NM,  tauCandidate, candPt2, candMass2, candCharge2);
	    findTrackByCharge(tracks02NM,  tauCandidate, candPt3, candMass3, candCharge3);
	  }
	  tree->Fill();
	}

}



// ------------ method called once each job just before starting event loop  ------------
void 
twoProngStudy::beginJob()
{
}


// ------------ method called once each job just after ending the event loop  ------------
	void 
twoProngStudy::endJob() 
{
}

void 
twoProngStudy::findTrackByCharge( std::vector<reco::TrackRef> tracks, reco::PFTauRef tauCandidate, float &candPt, float &candMass, int &candCharge){
  //Get charge of the original PFChargedHadronCands
  int tauCharge = 0;
  candPt = 0; 
  candMass = 0;
  for( const reco::PFCandidatePtr &chargedHadron: tauCandidate->signalPFChargedHadrCands()){
    tauCharge += chargedHadron->charge();
    //std::cout<<"signalPFChargedHadrCands Mass "<<chargedHadron->mass()<<std::endl;
  }

  //get real charge of tau...
  for(reco::TrackRef track : tracks){
    if(abs(track->charge()+tauCharge)==1){
      reco::Candidate::PolarLorentzVector badChargedHadron(track->pt(),track->eta(),track->phi(),.13957);
      std::cout<<"badChargedHadron mass "<<badChargedHadron.mass();
      candPt = (badChargedHadron+tauCandidate->p4()).pt();
      candMass = (badChargedHadron+tauCandidate->p4()).mass();
      candCharge = track->charge()+tauCharge;
      std::cout<<"Found a tau with 1 charge candPt "<<candPt<<" candMass "<<candMass<<std::endl;
      //return true;
      break;
    }
  }
  return;
}

void 
twoProngStudy::createVisTaus(std::vector<const reco::GenParticle*> GenObjects, std::vector <genVisTau_data>& genVisTaus){
  for(const reco::GenParticle* &genParticle : GenObjects){
    if(abs(genParticle->pdgId()) == 15){
      reco::Candidate::LorentzVector genVisTau(0,0,0,0);
      genVisTau = getVisMomentum(genParticle, GenObjects);      
      genVisTau_data temp;
      temp.numDaughters = genParticle->numberOfDaughters();
      temp.genTauPt = genParticle->pt();
      temp.p4=genVisTau;
      temp.charge=(-1)*(genParticle->pdgId()/15);
      genVisTaus.push_back(temp);
      //std::cout<<"Found Gen Vis tau"<<std::endl;
    }
  }
}

bool
twoProngStudy::foundGenTau(reco::PFTauRef tauCandidate, std::vector<const reco::GenParticle*> GenObjects, reco::GenParticle &matchedGenParticle){
  for(const reco::GenParticle* &genParticle : GenObjects){
    if(abs(genParticle->pdgId()) == 15){
      double deltaR = reco::deltaR(*tauCandidate,*genParticle);
      if(deltaR<0.3){
	matchedGenParticle = *genParticle;
	return true;
      }
    }
  }
  return false;
}

// finds the tauCandidate that most closely matches the gen vis tau
// returns the genVisTau_data which contains the four vector of the vis tau + the charge
bool
twoProngStudy::foundGenTauVis(reco::PFTauRef tauCandidate,  std::vector<genVisTau_data> genParticles, genVisTau_data &matchedGenParticle){
  for(const genVisTau_data genParticle: genParticles){
    double deltaR = reco::deltaR(*tauCandidate,genParticle.p4);
    if(deltaR<0.3){
      matchedGenParticle = genParticle;
      return true;
    }
  }
  return false;
}

bool
twoProngStudy::checkIfTrackMatchesPF(reco::PFTauRef tauCandidate, reco::TrackRef track){
  bool match = false;
  for( const reco::PFCandidatePtr &chargedHadron: tauCandidate->signalPFChargedHadrCands()){
    if(track == chargedHadron->trackRef()){
      match = true;
      break;
    }
  }
  return match;
}

reco::Candidate::LorentzVector 
  twoProngStudy::getVisMomentum(const reco::GenParticle* genLeg, std::vector<const reco::GenParticle*> genParticles)
{
  std::vector<const reco::GenParticle*> stableDaughters;
  findDaughters(genLeg, stableDaughters, 1);
  
  reco::Candidate::LorentzVector p4Vis = getVisMomentum(stableDaughters);

  return p4Vis;
}

void 
twoProngStudy::findDaughters(const reco::GenParticle* mother, std::vector<const reco::GenParticle*>& daughters, int status)
{
  unsigned numDaughters = mother->numberOfDaughters();
  //std::cout<<"numDaughters "<<numDaughters<<std::endl;
  for ( unsigned iDaughter = 0; iDaughter < numDaughters; ++iDaughter ) {
    const reco::GenParticle* daughter = mother->daughterRef(iDaughter).get();

    if ( status == -1 || daughter->status() == status ) daughters.push_back(daughter);

    findDaughters(daughter, daughters, status);
  }
}


reco::Candidate::LorentzVector 
twoProngStudy::getVisMomentum(const std::vector<const reco::GenParticle*>& daughters, int status)
{
  reco::Candidate::LorentzVector p4Vis(0,0,0,0);

  for ( std::vector<const reco::GenParticle*>::const_iterator daughter = daughters.begin();
	daughter != daughters.end(); ++daughter ) {
    if ( (status == -1 || (*daughter)->status() == status) && !isNeutrino(*daughter) ) {
      //std::cout << "adding daughter: pdgId = " << (*daughter)->pdgId() << ", Pt = " << (*daughter)->pt() << ","
      //	  << " eta = " << (*daughter)->eta() << ", phi = " << (*daughter)->phi()*180./TMath::Pi() << std::endl;
      p4Vis += (*daughter)->p4();
    }
  }

  //std::cout << "--> vis. Momentum: Pt = " << p4Vis.pt() << ", eta = " << p4Vis.eta() << ", phi = " << p4Vis.phi() << std::endl;

  return p4Vis;
}

bool 
twoProngStudy::isNeutrino(const reco::GenParticle* daughter)
{
  return ( TMath::Abs(daughter->pdgId()) == 12 || TMath::Abs(daughter->pdgId()) == 14 || TMath::Abs(daughter->pdgId()) == 16 );
}

// ------------ method called when starting to processes a run  ------------
/*
   void 
   twoProngStudy::beginRun(edm::Run const&, edm::EventSetup const&)
   {
   }
   */

// ------------ method called when ending the processing of a run  ------------
/*
   void 
   twoProngStudy::endRun(edm::Run const&, edm::EventSetup const&)
   {
   }
   */

// ------------ method called when starting to processes a luminosity block  ------------
/*
   void 
   twoProngStudy::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
   {
   }
   */

// ------------ method called when ending the processing of a luminosity block  ------------
/*
   void 
   twoProngStudy::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
   {
   }
   */

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
twoProngStudy::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	//The following says we do not know what parameters are allowed so do no validation
	// Please change this to state exactly what you do use, even if it is no parameters
	//  edm::ParameterSetDescription desc;
	//  desc.setUnknown();
	//  descriptions.addDefault(desc);
}

//define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(twoProngStudy);


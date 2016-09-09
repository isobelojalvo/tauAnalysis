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
#include <stdio.h>
#include <fstream>

// function declarations
void findPackedDaughters(const pat::PackedGenParticle* mother, std::vector<const pat::PackedGenParticle*>& daughters);

void findPackedDaughters(const pat::PackedGenParticle* mother, std::vector<const pat::PackedGenParticle*>& daughters)
{
    std::cout << "\nPacked Daughters: ";
    for(size_t i=0; i<mother->numberOfDaughters(); ++i){
        std::cout<< "status: " << mother->daughter(i)->status()<< " pdg: " << mother->daughter(i)->pdgId();
    }
}

// class declaration
class MiniAODtwoprong : public edm::EDAnalyzer {
    public:
        explicit MiniAODtwoprong(const edm::ParameterSet&);
        ~MiniAODtwoprong();

    private:
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

        // ----------member data ---------------------------
        edm::EDGetTokenT<reco::VertexCollection> vtxToken;
        edm::EDGetTokenT<pat::TauCollection> tauToken;
        edm::EDGetTokenT<std::vector <pat::PackedCandidate>> PFCandidateToken;
        std::string tauID;
        edm::EDGetTokenT<std::vector <reco::GenParticle> > prunedGenToken;
        edm::EDGetTokenT<std::vector <pat::PackedGenParticle> >packedGenToken;

        TTree* tree;
        Int_t nvtx;
        Float_t recoTauPt;
        Float_t recoTauEta;
        Float_t recoTauMass;
        Int_t recoDecayMode;
        Float_t tauPt;
        Float_t tauEta;
        Float_t tauMass;
        Int_t decayMode;
        Int_t leadPDG;
        Float_t pT1;
        Float_t dR1;
        Float_t pT2;
        Float_t dR2;
        Float_t pT3;
        Float_t dR3;
        Float_t dxy1;
        Float_t dxy2;
        Float_t dxy3;
        Float_t dz1;
        Float_t dz2;
        Float_t dz3;
        Float_t dxyErr1;
        Float_t dxyErr2;
        Float_t dxyErr3;
        Float_t dzErr1;
        Float_t dzErr2;
        Float_t dzErr3;
        Float_t numHits1;
        Float_t numHits2;
        Float_t numHits3;
        Float_t numPixHits1;
        Float_t numPixHits2;
        Float_t numPixHits3;
        Int_t   recoTrack;
        Float_t dxy;
        Float_t dxy_Sig;
        Float_t dxy_error;
        Float_t flightLengthSig;
        Int_t hasSecondaryVertex;
        Float_t gendxy3;
        Float_t gendz3;
        Float_t trackDR;
        Float_t trackDpT;
        Float_t gendxyErr3;
        Float_t gendzErr3;
        const reco::GenParticle* genTrack3;

};

MiniAODtwoprong::MiniAODtwoprong(const edm::ParameterSet& iConfig):
    vtxToken(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
    tauToken(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("taus"))),
    PFCandidateToken(consumes<std::vector<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("PFCandidates"))),
    prunedGenToken(consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("pruned"))),
    packedGenToken(consumes<std::vector<pat::PackedGenParticle> >(iConfig.getParameter<edm::InputTag>("packed")))
{
    tauID = iConfig.getParameter<std::string>("tauID");
    edm::Service<TFileService> fs;

    tree = fs->make<TTree>("Ntuple", "Ntuple");
    tree->Branch("nvtx",&nvtx,"nvtx/I");
    tree->Branch("recoTauPt", &recoTauPt,"recoTauPt/F");
    tree->Branch("recoTauEta", &recoTauEta,"recoTauEta/F");
    tree->Branch("recoTauMass",&recoTauMass,"recoTauMass/F");
    tree->Branch("recoDecayMode",&recoDecayMode,"recoDecayMode/I");
    tree->Branch("tauPt", &tauPt,"tauPt/F");
    tree->Branch("tauEta", &tauEta,"tauEta/F");
    tree->Branch("tauMass",&tauMass,"tauMass/F");
    tree->Branch("decayMode",&decayMode,"decayMode/I");
    tree->Branch("leadPDG",&leadPDG,"leadPDG/I");
    tree->Branch("pT1",&pT1,"pT1/F");
    tree->Branch("dR1",&dR1,"dR1/F");
    tree->Branch("pT2",&pT2,"pT2/F");
    tree->Branch("dR2",&dR2,"dR2/F");
    tree->Branch("pT3",&pT3,"pT3/F");
    tree->Branch("dR3",&dR3,"dR3/F");
    tree->Branch("dxy1",&dxy1,"dxy1/F");
    tree->Branch("dxy2",&dxy2,"dxy2/F");
    tree->Branch("dxy3",&dxy3,"dxy3/F");
    tree->Branch("dz1",&dz1,"dz1/F");
    tree->Branch("dz2",&dz2,"dz2/F");
    tree->Branch("dz3",&dz3,"dz3/F");
    tree->Branch("dxyErr1",&dxyErr1,"dxyErr1/F");
    tree->Branch("dxyErr2",&dxyErr2,"dxyErr2/F");
    tree->Branch("dxyErr3",&dxyErr3,"dxyErr3/F");
    tree->Branch("dzErr1",&dzErr1,"dzErr1/F");
    tree->Branch("dzErr2",&dzErr2,"dzErr2/F");
    tree->Branch("dzErr3",&dzErr3,"dzErr3/F");
    tree->Branch("numHits1",&numHits1,"numHits1/F");
    tree->Branch("numHits2",&numHits2,"numHits2/F");
    tree->Branch("numHits3",&numHits3,"numHits3/F");
    tree->Branch("numPixHits1",&numPixHits1,"numPixHits1/F");
    tree->Branch("numPixHits2",&numPixHits2,"numPixHits2/F");
    tree->Branch("numPixHits3",&numPixHits3,"numPixHits3/F");
    tree->Branch("recoTrack",&recoTrack,"recoTrack/I");
    tree->Branch("dxy",&dxy,"dxy/F");
    tree->Branch("dxy_Sig",&dxy_Sig,"dxy_Sig/F");
    tree->Branch("dxy_error",&dxy_error,"dxy_error/F");
    tree->Branch("flightLengthSig",&flightLengthSig,"flightLengthSig/F");
    tree->Branch("hasSecondaryVertex",&hasSecondaryVertex,"hasSecondaryVertex/I");
    tree->Branch("gendxy3",&gendxy3,"gendxy3/F");
    tree->Branch("gendz3",&gendz3,"gendz3/F");
    tree->Branch("trackDR",&trackDR,"trackDR/F");
    tree->Branch("trackDpT",&trackDpT,"trackDpT/F");
    tree->Branch("gendxyErr3",&gendxyErr3,"gendxyErr3/F");
    tree->Branch("gendzErr3",&gendzErr3,"gendzErr3/F");
}

MiniAODtwoprong::~MiniAODtwoprong()
{
}

    void

MiniAODtwoprong::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{	
    edm::Handle<reco::VertexCollection> vertices;
    iEvent.getByToken(vtxToken, vertices);
    nvtx=vertices->size();
    const reco::Vertex &PV = vertices->front();
    edm::Handle<pat::TauCollection> taus;
    iEvent.getByToken(tauToken, taus);
    edm::Handle<std::vector<reco::GenParticle> > genParticles;
    iEvent.getByToken(prunedGenToken, genParticles);
    edm::Handle<std::vector<pat::PackedGenParticle> > packedGenParticles;
    iEvent.getByToken(packedGenToken, packedGenParticles);
    edm::Handle<std::vector<pat::PackedCandidate>> PFCandidates;
    iEvent.getByToken(PFCandidateToken, PFCandidates);

    //Pruned generator leptons
    std::vector<const reco::GenParticle*> GenTaus;
    std::vector<const reco::GenParticle*> GenEles;
    std::vector<const reco::GenParticle*> GenMus;
    //Place generated leptons into separate lists
    for(std::vector<reco::GenParticle>::const_iterator genParticle = genParticles->begin(); genParticle != genParticles->end(); genParticle++){
        if(TMath::Abs(genParticle->pdgId()) == 11) GenEles.push_back(&(*genParticle));
        if(TMath::Abs(genParticle->pdgId()) == 13) GenMus.push_back(&(*genParticle));
        if(TMath::Abs(genParticle->pdgId()) == 15) GenTaus.push_back(&(*genParticle));
    }

    //Packed generator hadrons 
    std::vector<const pat::PackedGenParticle*> packedHadrons;
    for(std::vector<pat::PackedGenParticle>::const_iterator genParticle = packedGenParticles->begin(); genParticle != packedGenParticles->end(); genParticle++){
        if(TMath::Abs(genParticle->pdgId()) == 211) packedHadrons.push_back(&(*genParticle));
    }

    //Make list of possible PF charged pions to be used in tau reconstruction
    std::vector<const pat::PackedCandidate*> hadronCandidates;
    for(std::vector<pat::PackedCandidate>::const_iterator candidate= PFCandidates->begin(); candidate != PFCandidates->end(); candidate++){
        if(TMath::Abs(candidate->pdgId()) == 211) hadronCandidates.push_back(&(*candidate));
    }
    
    for (size_t i=0;i<GenTaus.size();i++){
        std::vector<const reco::GenParticle*> genTauDaughters;
        findDaughters(GenTaus[i], genTauDaughters);
        decayMode = GetDecayMode(genTauDaughters);
	    reco::Candidate::LorentzVector p4_vis = GetVisibleP4(genTauDaughters);  //only look at visible decay products of gen. tau
        tauPt = p4_vis.pt();
        tauEta = p4_vis.eta();
        tauMass = p4_vis.mass();
        if (decayMode/10==3 && tauPt>18 && TMath::Abs(tauEta)<2.3){  //3-prong requirement; pT and eta cuts   
            for(const pat::Tau & tau : *taus){
                recoDecayMode = tau.decayMode();
                recoTauPt = tau.pt();
                recoTauEta = tau.eta();
                recoTauMass = tau.mass();
                dxy = tau.dxy();
                dxy_error = tau.dxy_error();
                dxy_Sig = tau.dxy_Sig();
                flightLengthSig = tau.flightLengthSig();
                hasSecondaryVertex = tau.hasSecondaryVertex();
                int numSignalHadrons = 0;
                reco::CandidatePtr leadChargedHadr = tau.leadChargedHadrCand();
                reco::CandidatePtrVector signalCands = tau.signalCands();  //vector of the PF objects used in tau reconstruction
                for(size_t i=0; i<signalCands.size(); i++){
                    if (TMath::Abs(signalCands[i]->pdgId())==211) numSignalHadrons++;
                }
                if (recoTauPt>18 && TMath::Abs(recoTauEta)<2.3 && tau.tauID("decayModeFindingNewDMs")>0.5 && tau.tauID(tauID)>0.5 && (recoDecayMode==5 || recoDecayMode==6) && reco::deltaR(tau.eta(),tau.phi(),p4_vis.eta(),p4_vis.phi())<0.3){  //2-prong requirement (decay mode 5,6); dR gen. tau matching; lead track tagged as charged pion  

                    int n=1;//Used to distinguish the two signal pions when filling the tree
                    const pat::PackedCandidate* firstHadron = hadronCandidates[0];  //signal Candidates do not contain hit info, so we will need to find the equivalent packed candidate
                    const pat::PackedCandidate* secondHadron = hadronCandidates[0];
                    for(size_t j=0; j<signalCands.size(); j++){
                        if (TMath::Abs(signalCands[j]->pdgId())==211){  //There should be two pions used in reconstruction of a 2-prong tau
                            if (n==1){
                                pT1 = signalCands[j]->pt();
                                dR1 = reco::deltaR(signalCands[j]->phi(),signalCands[j]->eta(),tau.phi(),tau.eta());
                                dxyErr1 = signalCands[j]->dxyError();
                                dzErr1 = signalCands[j]->dzError();
                                for(size_t l=0; l<hadronCandidates.size(); l++){
                                    if (pT1 == hadronCandidates[l]->pt()) firstHadron = hadronCandidates[l];                               
                                }
                                dxy1 = firstHadron->dxy();
                                dz1 = firstHadron->dz();
                                numHits1 = firstHadron->numberOfHits();
                                numPixHits1 = firstHadron->numberOfPixelHits();
                            }
                            if (n==2){
                                pT2 = signalCands[j]->pt();
                                dR2 = reco::deltaR(signalCands[j]->phi(),signalCands[j]->eta(),tau.phi(),tau.eta());
                                dxyErr2 = signalCands[j]->dxyError();
                                dzErr2 = signalCands[j]->dzError();
                                for(size_t l=0; l<hadronCandidates.size(); l++){
                                    if (pT2 == hadronCandidates[l]->pt()) secondHadron = hadronCandidates[l];
                                }
                                dxy2 = secondHadron->dxy();
                                dz2 = secondHadron->dz();
                                numHits2 = secondHadron->numberOfHits();
                                numPixHits2 = secondHadron->numberOfPixelHits();
                            }
                            if (n==3) std::cout << "problem";
                            n++;
                        }
                    }
                    const pat::PackedCandidate* thirdHadron = findThirdHadron(hadronCandidates,signalCands,genTauDaughters, &recoTrack, &genTrack3, &trackDR);  //find nearest PF charged pion not already used in reconstruction
                    std::cout << "\nReco 3rd pT: "<< genTrack3->pt();
                    const pat::PackedGenParticle* thirdGenHadron = packedHadrons[0];
                    float minDiffpT = 1.0; 
                    for (size_t i=0; i<packedHadrons.size(); ++i){
                        if (TMath::Abs(genTrack3->pt()-packedHadrons[i]->pt()) < minDiffpT){
                            minDiffpT = TMath::Abs(genTrack3->pt()-packedHadrons[i]->pt());
                            thirdGenHadron = packedHadrons[i];
                        } 
                    }
                    trackDpT = TMath::Abs(thirdHadron->pt() - thirdGenHadron->pt());
                    gendxy3 = thirdGenHadron->dxy();
                    gendz3 = TMath::Abs(tau.vertex().z()-genTrack3->vz());
                    gendxyErr3 = genTrack3->dxyError();
                    gendzErr3 = genTrack3->dzError();
                    std::cout << " with gen pT of " << thirdGenHadron->pt();
                    if (recoTrack == 1){
                        pT3 = thirdHadron->pt();
                        dR3 = reco::deltaR(thirdHadron->phi(),thirdHadron->eta(),tau.phi(),tau.eta());
                        dxy3 = thirdHadron->dxy();
                        dz3 = thirdHadron->dz();
                        dxyErr3 = thirdHadron->dxyError();
                        dzErr3 = thirdHadron->dzError();
                        numHits3 = thirdHadron->numberOfHits();
                        numPixHits3 = thirdHadron->numberOfPixelHits();
                    }
                    else{
                        pT3 = genTrack3->pt();
    
                        dR3 = reco::deltaR(genTrack3->phi(),genTrack3->eta(),tau.phi(),tau.eta());
                        dxy3 = -1;//genTrack3->dxy();
                        dz3 = -1;//;genTrack3->dz();
                        //pat::genParticle genTrack3= pat::genParticle(genTrack3);
                        dxyErr3 = genTrack3->dxyError();
                        dzErr3 = genTrack3->dzError();
                        numHits3 = -1;
                        numPixHits3 = -1;
                    }
                    leadPDG = leadChargedHadr->pdgId();
                    tree->Fill();
                }
            }
        }
    }
}
//define this as a plug-in
DEFINE_FWK_MODULE(MiniAODtwoprong);

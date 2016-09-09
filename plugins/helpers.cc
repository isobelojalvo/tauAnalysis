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
#include "DataFormats/Provenance/interface/EventAuxiliary.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/TauReco/interface/PFTau.h"
#include "DataFormats/TauReco/interface/PFTauDiscriminator.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "helpers.h"
#include <iostream>
#include "TMath.h"
#include <vector>
#include <tgmath.h>

//List pdg and status of a given daughter vector
void listDaughters(std::vector<const reco::GenParticle*>& daughters){
    for (size_t i = 0; i < daughters.size(); ++i){
        std::cout << daughters[i]->pdgId() << " " << daughters[i]->status() << "; ";
    }
}

//Creates a vector of all (including intermediate) daughters for a given mother particle
void findDaughters(const reco::GenParticle* mother, std::vector<const reco::GenParticle*>& daughters)
{
    unsigned numDaughters = mother->numberOfDaughters();
    if (numDaughters == 0) std::cout << " none ";
    for (unsigned iDaughter = 0; iDaughter < numDaughters; ++iDaughter ) {
        const reco::GenParticle* daughter = mother->daughterRef(iDaughter).get();
        if (daughter->status() == 1){  //status = 1 is a final state daughter
            daughters.push_back(daughter); 
        }
        if (daughter->status() == 2){  //status = 2 is an intermediate daughter; will decay further
            daughters.push_back(daughter); 
            findDaughters(daughter, daughters);
        }
    }
}

void eraseHadronCands(std::vector<const pat::PackedCandidate*>& hadronCands, reco::CandidatePtrVector signalCands){
    for(size_t i=0; i<signalCands.size(); ++i){
        for(size_t j=0; j<hadronCands.size(); ++j){
            if(signalCands[i]->pt()==hadronCands[j]->pt()) hadronCands.erase(hadronCands.begin()+j);
        }
    }
}
 
//Finds closest charged pion not already used in 2-prong tau reconstruction
const pat::PackedCandidate* findThirdHadron(std::vector<const pat::PackedCandidate*> hadronCands, reco::CandidatePtrVector signalCands, std::vector<const reco::GenParticle*> daughters,Int_t *recoTrack, const reco::GenParticle* *genTrack3, Float_t *trackDR){
    const pat::PackedCandidate* thirdHadron = hadronCands[0];
    std::vector<const reco::GenParticle*> genTracks;  //make a list of charged pion daughters
    for(size_t i=0; i<daughters.size(); ++i){
        if (TMath::Abs(daughters[i]->pdgId())==211) genTracks.push_back(daughters[i]);
    }
    for(size_t i=0; i<signalCands.size(); ++i){  //loop through the generator charged pions
        if (TMath::Abs(signalCands[i]->pdgId())==211){
            float minDR = 1.0;
            int index = -1;
            for(size_t j=0; j<genTracks.size(); ++j){  //look for a signal candidate match
                float dR = reco::deltaR(genTracks[j]->eta(),genTracks[j]->phi(),signalCands[i]->eta(),signalCands[i]->phi());
                if (dR < minDR){
                    minDR = dR;
                    index = j;
                }
            }
            genTracks.erase(genTracks.begin()+index);  //remove the generator charged pion
        }
    }
    *genTrack3 = genTracks[0];
    eraseHadronCands(hadronCands,signalCands);
    float minDR = 1.0; //reusing minDR to match third track with hadron candidate
    for(size_t i=0; i<hadronCands.size(); ++i){
        if (hadronCands[i]->pdgId()==genTracks[0]->pdgId()){
            float dR = reco::deltaR(genTracks[0]->eta(),genTracks[0]->phi(),hadronCands[i]->eta(),hadronCands[i]->phi());
            if (dR < minDR && TMath::Abs(hadronCands[i]->pt()-genTracks[0]->pt()) < 5){
                minDR = dR;
                thirdHadron = hadronCands[i];
            }   
        }
    }
    *trackDR = minDR;
    if (minDR > 0.02) *recoTrack = 0;
    else *recoTrack = 1;
    std::cout <<"\n\nThird reco track by dR: dPT = " << TMath::Abs(thirdHadron->pt()-genTracks[0]->pt()) << " dR = " << reco::deltaR(thirdHadron->eta(),thirdHadron->phi(),genTracks[0]->eta(),genTracks[0]->phi());     
    std::cout <<"\nthird pt: " <<thirdHadron->pt() << " gen pt: " <<genTracks[0]->pt();
    
    return thirdHadron;
}

        
        

//Checks if the reconstructed particle is a neutrino (returns 1 if it is a neutrino)
bool isNeutrino(const reco::Candidate* daughter)
{
  return (TMath::Abs(daughter->pdgId()) == 12 || TMath::Abs(daughter->pdgId()) == 14 || TMath::Abs(daughter->pdgId()) == 16 || TMath::Abs(daughter->pdgId()) == 18);
}

//Returns a decay mode for a generator tau (input its vector of daughters)
int GetDecayMode(std::vector<const reco::GenParticle*>& daughters){
	int decayMode = -1;
	std::vector<int> counts(4,0);
    for(size_t i=0; i<daughters.size(); ++i){
		int pdg = daughters[i]->pdgId();
		if (TMath::Abs(pdg)==11) ++counts[0];  //electrons
		if (TMath::Abs(pdg)==13) ++counts[1];  //muons
		if (TMath::Abs(pdg)==111) ++counts[2];  //neutral pions
        if (TMath::Abs(pdg)==211) ++counts[3];  //charged pions
 	}
	if (counts[0] > 0) decayMode = 3;
	if (counts[1] > 0) decayMode = 4;
	if (counts[3] > 0) decayMode = 10*counts[3]+counts[2];  //First digit is # of prongs (1 or 3), second is # of neutral pions
	return decayMode;
}

//Gets visible 4-momentum of a particle from list of daughters
reco::Candidate::LorentzVector GetVisibleP4(std::vector<const reco::GenParticle*>& daughters){
	reco::Candidate::LorentzVector p4_vis(0,0,0,0);	
 	for(size_t i = 0; i < daughters.size(); ++i){
 		if (!isNeutrino(daughters[i]) && daughters[i]->status() == 1){  //list should include intermediate daughters, so check for status = 1
 			p4_vis += daughters[i]->p4();
		}
 	}
	return p4_vis;
}


//Checks if the generator-level particle (tau) decays to hadrons (returns 1 if it decays hadronically)
bool isHadronic(const reco::GenParticle* tau){
	bool isHadronic = 1;
	for(size_t j = 0; j < tau->numberOfDaughters(); ++j){ //Loop through daughters of gen. tau
		if (TMath::Abs(tau->daughter(j)->pdgId()) == 11 || TMath::Abs(tau->daughter(j)->pdgId()) == 13 || TMath::Abs(tau->daughter(j)->pdgId()) == 15 || TMath::Abs(tau->daughter(j)->pdgId()) == 321){ //Check if the daughter is another lepton
			isHadronic = 0;
		}
	}
	return isHadronic;
}

reco::PFJetRef getJetRef(const reco::PFTau& tau) {
	if (tau.jetRef().isNonnull())
		return tau.jetRef();
	else if (tau.pfTauTagInfoRef()->pfjetRef().isNonnull())
		return tau.pfTauTagInfoRef()->pfjetRef();
	else throw cms::Exception("cant find jet ref");
}

reco::PFJetRef getJetRef(const pat::Tau tau) {
        if (tau.pfJetRef().isNonnull())
                return tau.pfJetRef();
        else throw cms::Exception("cant find jet ref");
}

bool genMatchingMiniAOD(const pat::Tau tau, std::vector<const reco::GenParticle*>& GenPart, double maxDR) {
	bool tau_match=false;
	for (size_t i = 0; i < GenPart.size(); ++i) {
		if (abs(GenPart[i]->pdgId())==15){
			double deltaR = reco::deltaR(tau, *GenPart[i]);
			if (deltaR < maxDR) {
				tau_match=true;
			}
		}
	}
	return tau_match;
}

std::vector<const reco::GenParticle*> getGenParticleCollectionMiniAOD(const edm::Event& evt) {
	std::vector<const reco::GenParticle*> output;
	edm::Handle< std::vector<reco::GenParticle> > handle;
	evt.getByLabel("prunedGenParticles", handle);
	// Loop over objects in current collection
	for (size_t j = 0; j < handle->size(); ++j) {
		const reco::GenParticle& object = handle->at(j);
		if(abs(object.pdgId()) == 15) output.push_back(&object);
	}
	return output;
}

std::vector<const reco::GenParticle*> getGenEleCollectionMiniAOD(const edm::Event& evt) {
        std::vector<const reco::GenParticle*> output;
        edm::Handle< std::vector<reco::GenParticle> > handle;
        evt.getByLabel("prunedGenParticles", handle);
        // Loop over objects in current collection
        for (size_t j = 0; j < handle->size(); ++j) {
                const reco::GenParticle& object = handle->at(j);
                if(abs(object.pdgId()) == 11) output.push_back(&object);
        }
        return output;
}

std::vector<const reco::GenParticle*> getGenMuCollectionMiniAOD(const edm::Event& evt) {
        std::vector<const reco::GenParticle*> output;
        edm::Handle< std::vector<reco::GenParticle> > handle;
        evt.getByLabel("prunedGenParticles", handle);
        // Loop over objects in current collection
        for (size_t j = 0; j < handle->size(); ++j) {
                const reco::GenParticle& object = handle->at(j);
                if(abs(object.pdgId()) == 13) output.push_back(&object);
        }
        return output;
}

// Get collection of generator particles with status 2
std::vector<const reco::GenParticle*> getGenParticleCollection(const edm::Event& evt) {
	std::vector<const reco::GenParticle*> output;
	edm::Handle< std::vector<reco::GenParticle> > handle;
	evt.getByLabel("genParticles", handle);
	// Loop over objects in current collection
	for (size_t j = 0; j < handle->size(); ++j) {
		const reco::GenParticle& object = handle->at(j);
		//if(fabs(object.pdgId())==15 && object.status() == 2) output.push_back(&object);
		if(abs(object.pdgId()) == 15) output.push_back(&object);
	}
	return output;
}

std::vector<const reco::GenParticle*> getGenEleCollection(const edm::Event& evt) {
        std::vector<const reco::GenParticle*> output;
        edm::Handle< std::vector<reco::GenParticle> > handle;
        evt.getByLabel("genParticles", handle);
        // Loop over objects in current collection
        for (size_t j = 0; j < handle->size(); ++j) {
                const reco::GenParticle& object = handle->at(j);
                //if(fabs(object.pdgId())==15 && object.status() == 2) output.push_back(&object);
                if(abs(object.pdgId()) == 11) output.push_back(&object);
        }
        return output;
}

std::vector<const reco::GenParticle*> getGenMuCollection(const edm::Event& evt) {
        std::vector<const reco::GenParticle*> output;
        edm::Handle< std::vector<reco::GenParticle> > handle;
        evt.getByLabel("genParticles", handle);
        // Loop over objects in current collection
        for (size_t j = 0; j < handle->size(); ++j) {
                const reco::GenParticle& object = handle->at(j);
                //if(fabs(object.pdgId())==15 && object.status() == 2) output.push_back(&object);
                if(abs(object.pdgId()) == 13) output.push_back(&object);
        }
        return output;
}


// Method to find the best match between tag tau and gen object. The best matched gen tau object will be returned. If there is no match within a DR < 0.5, a null pointer is returned
//const reco::GenParticle* findBestGenMatch1(const reco::PFTau TagTauObj,
const reco::GenParticle* findBestGenMatch(const reco::PFTau& tauObj,
		std::vector<const reco::GenParticle*>& GenPart, double maxDR) {
	const reco::GenParticle* output = NULL;
	double bestDeltaR = maxDR;
	for (size_t i = 0; i < GenPart.size(); ++i) {
		double deltaR = reco::deltaR(tauObj, *GenPart[i]);
		if (deltaR < maxDR) {
			if (deltaR < bestDeltaR) {
				output = GenPart[i];
				bestDeltaR = deltaR;
			}
		}
	}
	return output;
}

const reco::GenParticle* findBestGenMatch(const pat::Tau& tauObj,
                std::vector<const reco::GenParticle*>& GenPart, double maxDR) {
        const reco::GenParticle* output = NULL;
        double bestDeltaR = maxDR;
        for (size_t i = 0; i < GenPart.size(); ++i) {
                double deltaR = reco::deltaR(tauObj, *GenPart[i]);
                if (deltaR < maxDR) {
                        if (deltaR < bestDeltaR) {
                                output = GenPart[i];
                                bestDeltaR = deltaR;
                        }
                }
        }
        return output;
}

int findBestGenMatchIndex(const pat::Tau& tauObj,
                std::vector<const reco::GenParticle*>& GenPart, double maxDR) {
        double bestDeltaR = maxDR;
	int index = -1;
        for (size_t i = 0; i < GenPart.size(); ++i) {
                double deltaR = reco::deltaR(tauObj, *GenPart[i]);
                if (deltaR < maxDR) {
                        if (deltaR < bestDeltaR) {
                                bestDeltaR = deltaR;
                        }
                }
        }
	return index;
}

const pat::Jet* findBestJetMatch(const pat::Tau& tauObj,
                std::vector<const pat::Jet*>& jet_denom_vec, double maxDR) {
        const pat::Jet* output = NULL;
        double bestDeltaR = maxDR;
        for (size_t i = 0; i < jet_denom_vec.size(); ++i) {
                double deltaR = reco::deltaR(tauObj, *jet_denom_vec[i]);
                if (deltaR < maxDR) {
                        if (deltaR < bestDeltaR) {
                                output = jet_denom_vec[i];
                                bestDeltaR = deltaR;
                        }
                }
        }
        return output;
}

bool isLooseJet(const reco::PFJet jet){
	bool loose = true;
	if (jet.neutralHadronEnergyFraction() >= 0.99) loose = false;
	if (jet.neutralEmEnergyFraction() >= 0.99) loose = false;
	if (jet.numberOfDaughters() <= 1) loose = false; //getPFConstitutents broken in miniAOD
	if (std::abs(jet.eta()) < 2.4) {
		if (jet.chargedHadronEnergyFraction() == 0) loose = false;
		if (jet.chargedHadronMultiplicity() == 0) loose = false;
		if (jet.chargedEmEnergyFraction() >= 0.99) loose = false;
	}
	return loose;
}
bool isMediumJet(const reco::PFJet jet){
	bool medium = true;
	if (jet.neutralHadronEnergyFraction() >= 0.95) medium = false;
	if (jet.neutralEmEnergyFraction() >= 0.95) medium = false;
	if (jet.numberOfDaughters() <= 1) medium = false; //getPFConstitutents broken in miniAOD
	if (std::abs(jet.eta()) < 2.4) {
		if (jet.chargedHadronEnergyFraction() == 0) medium = false;
		if (jet.chargedHadronMultiplicity() == 0) medium = false;
		if (jet.chargedEmEnergyFraction() >= 0.99) medium = false;
	}
	return medium;
}

bool isTightJet(const reco::PFJet jet){
	bool tight = true;
	if (jet.neutralHadronEnergyFraction() >= 0.90) tight = false;
	if (jet.neutralEmEnergyFraction() >= 0.90) tight = false;
	if (jet.numberOfDaughters() <= 1) tight = false; //getPFConstitutents broken in miniAOD
	if (std::abs(jet.eta()) < 2.4) {
		if (jet.chargedHadronEnergyFraction() == 0) tight = false;
		if (jet.chargedHadronMultiplicity() == 0) tight = false;
		if (jet.chargedEmEnergyFraction() >= 0.99) tight = false;
	}
	return tight;
}

bool isLooseJet(const pat::Jet jet){
        bool loose = true;
        if (jet.neutralHadronEnergyFraction() >= 0.99) loose = false;
        if (jet.neutralEmEnergyFraction() >= 0.99) loose = false;
        if (jet.numberOfDaughters() <= 1) loose = false; //getPFConstitutents broken in miniAOD
        if (std::abs(jet.eta()) < 2.4) {
                if (jet.chargedHadronEnergyFraction() == 0) loose = false;
                if (jet.chargedHadronMultiplicity() == 0) loose = false;
                if (jet.chargedEmEnergyFraction() >= 0.99) loose = false;
        }
        return loose;
}
bool isMediumJet(const pat::Jet jet){
        bool medium = true;
        if (jet.neutralHadronEnergyFraction() >= 0.95) medium = false;
        if (jet.neutralEmEnergyFraction() >= 0.95) medium = false;
        if (jet.numberOfDaughters() <= 1) medium = false; //getPFConstitutents broken in miniAOD
        if (std::abs(jet.eta()) < 2.4) {
                if (jet.chargedHadronEnergyFraction() == 0) medium = false;
                if (jet.chargedHadronMultiplicity() == 0) medium = false;
                if (jet.chargedEmEnergyFraction() >= 0.99) medium = false;
        }
        return medium;
}

bool isTightJet(const pat::Jet jet){
        bool tight = true;
        if (jet.neutralHadronEnergyFraction() >= 0.90) tight = false;
        if (jet.neutralEmEnergyFraction() >= 0.90) tight = false;
        if (jet.numberOfDaughters() <= 1) tight = false; //getPFConstitutents broken in miniAOD
        if (std::abs(jet.eta()) < 2.4) {
                if (jet.chargedHadronEnergyFraction() == 0) tight = false;
                if (jet.chargedHadronMultiplicity() == 0) tight = false;
                if (jet.chargedEmEnergyFraction() >= 0.99) tight = false;
        }
        return tight;
}


/*
 * =====================================================================================
 *
 *       Filename:  Helpers.h
 *
 *    Description:  Common UCT functions.
 *
 *         Author:  M. Cepeda, S. Dasu, E. Friis
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#ifndef HELPERS_W9QK6HND
#define HELPERS_W9QK6HND
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
//MINIAOD
void findDaughters(const reco::GenParticle* mother, std::vector<const reco::GenParticle*>& daughters);
//void findPackedDaughters(const pat::PackedGenParticle* mother, std::vector<const pat::PackedGenParticle*>& daughters);
void listDaughters(std::vector<const reco::GenParticle*>& daughters);
void eraseHadronCands(std::vector<const pat::PackedCandidate*>& hadronCands, reco::CandidatePtrVector signalCands);
const pat::PackedCandidate* findThirdHadron(std::vector<const pat::PackedCandidate*> hadronCands, reco::CandidatePtrVector signalCands, std::vector<const reco::GenParticle*> daughters, Int_t *recoTrack, const reco::GenParticle* *genTrack3, Float_t *trackDR);
void GetDaughterDecayMode(const reco::Candidate* particle, std::vector<int> &counts);
int GetDecayMode(std::vector<const reco::GenParticle*>& daughters);
//int GetPackedDecayMode(std::vector<const pat::PackedGenParticle*>& daughters);
std::vector<const reco::GenParticle*> getGenParticleCollectionMiniAOD(const edm::Event& evt);
std::vector<const reco::GenParticle*> getGenEleCollectionMiniAOD(const edm::Event& evt);
std::vector<const reco::GenParticle*> getGenMuCollectionMiniAOD(const edm::Event& evt);
bool genMatchingMiniAOD(const pat::Tau tau, std::vector<const reco::GenParticle*>& GenPart, double maxDR);
bool isHadronic(const reco::GenParticle* tau);
reco::Candidate::LorentzVector GetVisibleP4(std::vector<const reco::GenParticle*>& daughters);
//reco::Candidate::LorentzVector GetPackedVisibleP4(std::vector<const pat::PackedGenParticle*>& daughters);
bool isNeutrino(const reco::Candidate* daughter);
//AODSIM
reco::PFJetRef getJetRef(const reco::PFTau& tau);
std::vector<const reco::GenParticle*> getGenParticleCollection(const edm::Event& evt);	
const reco::GenParticle* findBestGenMatch(const reco::PFTau& TagTauObj,std::vector<const reco::GenParticle*>& GenPart, double maxDR);
bool isLooseJet(const reco::PFJet jet);
bool isMediumJet(const reco::PFJet jet);
bool isTightJet(const reco::PFJet jet);
bool isLooseJet(const pat::Jet jet);
bool isMediumJet(const pat::Jet jet);
bool isTightJet(const pat::Jet jet);
const reco::GenParticle* findBestGenMatch(const pat::Tau& tauObj,std::vector<const reco::GenParticle*>& GenPart, double maxDR);
const pat::Jet* findBestJetMatch(const pat::Tau& tauObj,std::vector<const pat::Jet*>& jet_denom_vec, double maxDR);
int findBestGenMatchIndex(const pat::Tau& tauObj,std::vector<const reco::GenParticle*>& GenPart, double maxDR);
#endif

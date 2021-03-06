//-------------------------------------------------------
// Description:
// Runs Hgg analysis
// Assumes all h->gg
// "fakes" one h->bbar
// Authors: 
//-------------------------------------------------------


#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <TTree.h>
#include "CMS/CMSRazorHggHbb.hh"
#include <fastjet/tools/Pruner.hh>

CMSRazorHggHbb::CMSRazorHggHbb(TTree *tree, double Lumi, string analysis) : CMSReco(tree) {
  _Lumi = Lumi;
  _statTools = new StatTools(-99);
  _analysis = analysis;
}

CMSRazorHggHbb::~CMSRazorHggHbb(){
}

void CMSRazorHggHbb::SetSqrts(double sqrts) {
  _sqrts = sqrts;
}

// loop over events - real analysis
void CMSRazorHggHbb::Loop(string outFileName) {
  srand(time(0)); 
  if(DetectorBase::fChain == 0) return;

  double MR, RSQ;

  double jetPt[50];
  double jetEta[50];
  double jetPhi[50];

  double pho1Pt, pho2Pt;
  double pho1Eta, pho2Eta;
  double pho1Phi, pho2Phi;

  double bjet1Pt = -1, bjet2Pt = -1;
  double bjet1Eta = 999., bjet2Eta = 999.;
  double bjet1Phi = -999., bjet2Phi = -999.;

  double higgsPt, higgsEta, higgsPhi, higgsMass;
  int numJets;
  double MET;
  
  //new variables
  int numBJets;
  int numPhotons;
  int numBox = -1;
  int jetLen = 100;
  int Hem1_csts[jetLen], Hem2_csts[jetLen];
  double dPhi, dPhiHiggsJ1, dPhiHiggsJ2;
  int leadingHemContents, subleadingHemContents;
  int numGenEvents;
  double pthat = -1;
  int numSbottoms;
  int numHgg=0;
  double neutMET;
  double sbottomPt[2];
  double sbottomEta[2];
  double sbottomPhi[2];
  double lspPt[2];
  double lspEta[2];
  double lspPhi[2];


  // Open Output file
  TFile *file = new TFile(outFileName.c_str(),"UPDATE");

  TTree* outTree = new TTree("RazorInclusive","RazorInclusive");
  outTree->Branch("MR", &MR, "MR/D");
  outTree->Branch("RSQ", &RSQ, "RSQ/D");
  outTree->Branch("numJets", &numJets, "numJets/I");
  outTree->Branch("numPhotons", &numPhotons, "numPhotons/I");
  outTree->Branch("jetPt", jetPt, "jetPt[numJets]/D");
  outTree->Branch("jetEta", jetEta, "jetEta[numJets]/D");
  outTree->Branch("jetPhi", jetPhi, "jetPhi[numJets]/D");

  outTree->Branch("pho1Pt", &pho1Pt, "pho1Pt/D");
  outTree->Branch("pho1Eta", &pho1Eta, "pho1Eta/D");
  outTree->Branch("pho1Phi", &pho1Phi, "pho1Phi/D");
  outTree->Branch("pho2Pt", &pho2Pt, "pho2Pt/D");
  outTree->Branch("pho2Eta", &pho2Eta, "pho2Eta/D");
  outTree->Branch("pho2Phi", &pho2Phi, "pho2Phi/D");
  outTree->Branch("higgsPt", &higgsPt, "higgsPt/D");
  outTree->Branch("higgsEta", &higgsEta, "higgsEta/D");
  outTree->Branch("higgsPhi", &higgsPhi, "higgsPhi/D");
  outTree->Branch("higgsMass", &higgsMass, "higgsMass/D");

  outTree->Branch("numBox", &numBox, "numBox/I");
  outTree->Branch("MET", &MET, "MET/D");

  // added branches
  outTree->Branch("dPhi", &dPhi, "dPhi/D");
  outTree->Branch("dPhiHiggsJ1", &dPhiHiggsJ1, "dPhiHiggsJ1/D");
  outTree->Branch("dPhiHiggsJ2", &dPhiHiggsJ2, "dPhiHiggsJ2/D");
  outTree->Branch("numBJets", &numBJets, "numBJets/I");
  outTree->Branch("leadingHemContents", &leadingHemContents, "leadingHemContents/I");
  outTree->Branch("subleadingHemContents", &subleadingHemContents, "subleadingHemContents/I"); //0 if higgs only, //1 if jets only, //2 if higgs and jets
  outTree->Branch("pthat", &pthat, "pthat/D");
  outTree->Branch("sbottomPt", &sbottomPt, "sbottomPt[2]/D");
  outTree->Branch("sbottomEta", &sbottomEta, "sbottomEta[2]/D");
  outTree->Branch("sbottomPhi", &sbottomPhi, "sbottomPhi[2]/D");
  outTree->Branch("numSbottoms", &numSbottoms, "numSbottoms/I");
  outTree->Branch("numHgg", &numHgg, "numHgg/I");
  outTree->Branch("neutMET", &neutMET, "neutMET/D");
  outTree->Branch("lspPt", &lspPt, "lspPt[2]/D");
  outTree->Branch("lspEta", &lspEta, "lspEta[2]/D");
  outTree->Branch("lspPhi", &lspPhi, "lspPhi[2]/D");
  outTree->Branch("pthat", &pthat, "pthat/D");

  // loop over entries
  Long64_t nbytes = 0, nb = 0;
  Long64_t nentries = DetectorBase::fChain->GetEntries();
  std::cout << "Number of entries = " << nentries << std::endl;

  // set the by-event weight
  for (Long64_t jentry=0; jentry<nentries;jentry+=1) {

    if(verbose) cout << "new event" << endl;

    // clean physics-objects blocks
    CleanEvent();

    // get new event
    Long64_t ientry = DetectorBase::LoadTree(jentry);
    if (ientry < 0) break;
    nb = DetectorBase::fChain->GetEntry(jentry);   nbytes += nb;
    if (jentry%1 == 0) std::cout << ">>> Processing event # " << jentry << std::endl;

    //reset jet variables
    numJets = 0;
    numBJets = 0;
    numPhotons = 0;
    for(int i = 0; i < 50; i++){
        jetPt[i] = -1;
        jetEta[i] = -999;
        jetPhi[i] = -999;
    }
    pho1Pt = 0; pho1Eta = -999; pho1Phi = -999;
    pho2Pt = 0; pho2Eta = -999; pho2Phi = -999;
    higgsPt = 0; higgsEta = -999; higgsPhi = -999;
    higgsMass = -999;
    for(int i = 0; i < 2; i++){
      sbottomPt[i] = -999;
      sbottomEta[i] = -999;
      sbottomPhi[i] = -999;
      lspPt[i] =  -999;
      lspEta[i] = -999;
      lspPhi[i] = -999;
    }


    TLorentzVector sbottomvector1;
    TLorentzVector sbottomvector2;
    numSbottoms = 0;
    numHgg = 0;

    int numLSP = 0;
    TLorentzVector LSP1;
    TLorentzVector LSP2;

    // THIS PART SAVES SBOTTOM + LSP INFORMATION

    cout << "SUSY: " << SUSY << " GenTreeParticle: " << GenTreeParticle <<  endl;
    for(int iSUSY = 0; iSUSY < SUSY; iSUSY++){
      cout << "SUSYId: " << SUSYPdgId[iSUSY] << endl;
      cout << "SUSYD1Id: " << SUSYD1PdgId[iSUSY] << endl;
      if (fabs(SUSYPdgId[iSUSY])==1000005 && (SUSYD1PdgId[iSUSY]==1000023 || SUSYD1PdgId[iSUSY]==1000022)) {
	numSbottoms++;
	if (numSbottoms == 1){
	  sbottomvector1.SetPxPyPzE(SUSYPx[iSUSY], SUSYPy[iSUSY], SUSYPz[iSUSY], SUSYE[iSUSY]);
	  //	  cout << "Px1: " << sbottomvector1.Px() << endl;
	}
	else{
	  sbottomvector2.SetPxPyPzE(SUSYPx[iSUSY], SUSYPy[iSUSY], SUSYPz[iSUSY], SUSYE[iSUSY]);
	  //          cout << "Px2: " << sbottomvector2.Px() << endl;
	}
      }
      if (fabs(SUSYPdgId[iSUSY])==1000022) {
	numLSP++;
	if (numLSP == 1){
	  LSP1.SetPxPyPzE(SUSYPx[iSUSY], SUSYPy[iSUSY], SUSYPz[iSUSY], SUSYE[iSUSY]);
	}
	else{
	  LSP2.SetPxPyPzE(SUSYPx[iSUSY], SUSYPy[iSUSY], SUSYPz[iSUSY], SUSYE[iSUSY]);
	}
      }
    }

    lspPt[0] = LSP1.Pt();
    lspPt[1] = LSP2.Pt();
    lspEta[0] = LSP1.Eta();
    lspEta[1] = LSP2.Eta();
    lspPhi[0] = LSP1.Phi();
    lspPhi[1] = LSP2.Phi();

    TLorentzVector LSPs = LSP1 + LSP2;
    neutMET = LSPs.Pt();

    for(int iGenTreeParticle = 0; iGenTreeParticle < GenTreeParticle; iGenTreeParticle++){
      //if (GenTreeParticlePdgId[iGenTreeParticle]==25) genNumHiggs++;
      /*if (GenTreeParticlePdgId[iGenTreeParticle]==22 && GenTreeParticleM1PdgId[iGenTreeParticle]==25) numHgg++;
      if (GenTreeParticlePdgId[iGenTreeParticle]==1000005){
        sbottomvector1.SetPxPyPzE(GenTreeParticlePx[iGenTreeParticle], GenTreeParticlePy[iGenTreeParticle], GenTreeParticlePz[iGenTreeParticle], GenTreeParticleE[iGenTreeParticle]);
	numSbottoms++;
      }
      if (GenTreeParticlePdgId[iGenTreeParticle]==-1000005){
        sbottomvector2.SetPxPyPzE(GenTreeParticlePx[iGenTreeParticle], GenTreeParticlePy[iGenTreeParticle], GenTreeParticlePz[iGenTreeParticle], GenTreeParticleE[iGenTreeParticle]);
	numSbottoms++;
	}*/
    }

    TLorentzVector sbottoms = sbottomvector1 + sbottomvector2;
    sbottomPt[0] = sbottomvector1.Pt();
    sbottomPt[1] = sbottomvector2.Pt();
    sbottomEta[0] = sbottomvector1.Eta();
    sbottomEta[1] = sbottomvector2.Eta();
    sbottomPhi[0] = sbottomvector1.Phi();
    sbottomPhi[1] = sbottomvector2.Phi();
    pthat = sbottoms.Pt();

    //    cout << "pthat: " << pthat << endl;

    // Build the event at generator level
    PFReco();
    vector<fastjet::PseudoJet> empty;
    vector<fastjet::PseudoJet> JetsConst = PFJetConstituents(empty,empty,empty); //note that this I think includes photons

    // alternative might be 
    /*for(int iPhot = 0; iPhot < _PFPhotons.size(); iPhot++){
      vector<fastjet::PseudoJet> JetsConst = PFJetConstituents(_PFPhotons[iPhot],empty,empty);
      }*/
    // if you do this you need to adjust how numJets is defined (add 2 if there are two higgs found)

    //Photons 
    double bestSumPt = -1;
    int bestPhotIndex1 = -1;
    int bestPhotIndex2 = -1;
    double bestMass = -1;
    int numHiggs = 0;
    double secondBestSumPt = -1;
    int secondBestPhotIndex1 = -1;
    int secondBestPhotIndex2 = -1;
    double secondBestMass = -1; 
    numBox = -1;

    //loop over all pairs of gen photons and find the pair closest to Higgs mass
    for(int iPhot = 0; iPhot < _PFPhotons.size(); iPhot++){
      if(_PFPhotons[iPhot].pt() < 25) continue; //only count photons that are > 25 GeV
      //    cout<< "found a photon" <<endl;
      numPhotons++;
      if(_PFPhotons[iPhot].pt() < 40) continue; //at least one photon is 40 GeV
      for(int jPhot = 0; jPhot < _PFPhotons.size(); jPhot++){
	if(iPhot == jPhot) continue;
	//if(_PFPhotons[jPhot].pt() < 25 || abs(_PFPhotons[jPhot].eta()) > 1.44) continue;
	fastjet::PseudoJet higgsCandidate = _PFPhotons[iPhot] + _PFPhotons[jPhot];
	//check if this pair is in the correct mass range
	if(higgsCandidate.m() > 100){
	  cout << "Checking!" << endl;
	  numHiggs++;
	  if(_PFPhotons[iPhot].pt() + _PFPhotons[jPhot].pt() > bestSumPt){
	    cout << "Made it!" << endl;
	    secondBestMass = bestMass;
	    secondBestSumPt = bestSumPt;
	    secondBestPhotIndex1 = bestPhotIndex1;
	    secondBestPhotIndex2 = bestPhotIndex2;
	    bestMass = higgsCandidate.m();
	    bestSumPt = _PFPhotons[iPhot].pt() + _PFPhotons[jPhot].pt();
	    bestPhotIndex1 = iPhot;
	    bestPhotIndex2 = jPhot;
	  }
	}
      }
    }


    double randNum = ((double)rand()/(RAND_MAX)); //random number between 0 and 1
    //    cout << "Random Number 1: " << randNum << endl;
    bool firstPhot = true; //bool is true if there is only one photon or if the first diphoton pair is set as diphotons, if False we set first diphoton pair to be a "bb"
    bool twoHiggs = false;
    if (bestPhotIndex1 == -1){
      //      cout << "Didn't find a higgs candidate" << endl;
            continue; ///continue if we didn't find a higgs candidate
    }
    cout << "Found higgs candidate!!!!" << endl;

    if (secondBestPhotIndex1 > -1){ //two higgs candidates found
      twoHiggs = true;
      if (randNum > 0.5){
	firstPhot = false;
      }
    }
    fastjet::PseudoJet bestDiphoton;
    if (firstPhot){ //pair#1 is diphoton
      bestDiphoton = _PFPhotons[bestPhotIndex1] + _PFPhotons[bestPhotIndex2];
      if(bestDiphoton.pt() < 20 || fabs(_PFPhotons[bestPhotIndex1].eta()) > 1.44 || fabs(_PFPhotons[bestPhotIndex2].eta()) > 1.44){
        cout << "Diphoton system has pT too small or one of the photons is not in ECAL barrel" << endl; 
	continue;
      }
    }
    else { //pair #2 is diphoton
      bestDiphoton = _PFPhotons[secondBestPhotIndex1] + _PFPhotons[secondBestPhotIndex2];
      if(bestDiphoton.pt() < 20 || fabs(_PFPhotons[secondBestPhotIndex1].eta()) > 1.44 || fabs(_PFPhotons[secondBestPhotIndex2].eta()) > 1.44){
        cout << "Diphoton system has pT too small or one of the photons is not in ECAL barrel" << endl; 
	continue;
      }
    }

    std::cout << "-->pass1" << std::endl;
    //fill photon and bjet variables
    if (firstPhot && twoHiggs){
      pho1Pt = _PFPhotons[bestPhotIndex1].pt();
      pho1Eta = _PFPhotons[bestPhotIndex1].eta();
      pho1Phi = _PFPhotons[bestPhotIndex1].phi();
      pho2Pt = _PFPhotons[bestPhotIndex2].pt();
      pho2Eta = _PFPhotons[bestPhotIndex2].eta();
      pho2Phi = _PFPhotons[bestPhotIndex2].phi();
      bjet1Pt = _PFPhotons[secondBestPhotIndex1].pt();
      bjet1Eta = _PFPhotons[secondBestPhotIndex1].eta();
      bjet1Phi = _PFPhotons[secondBestPhotIndex1].phi();
      bjet2Pt = _PFPhotons[secondBestPhotIndex2].pt();
      bjet2Eta = _PFPhotons[secondBestPhotIndex2].eta();
      bjet2Phi = _PFPhotons[secondBestPhotIndex2].phi();
    }
    else if (firstPhot){
      pho1Pt = _PFPhotons[bestPhotIndex1].pt();
      pho1Eta = _PFPhotons[bestPhotIndex1].eta();
      pho1Phi = _PFPhotons[bestPhotIndex1].phi();
      pho2Pt = _PFPhotons[bestPhotIndex2].pt();
      pho2Eta = _PFPhotons[bestPhotIndex2].eta();
      pho2Phi = _PFPhotons[bestPhotIndex2].phi();
    }
    else {
      pho1Pt = _PFPhotons[secondBestPhotIndex1].pt();
      pho1Eta = _PFPhotons[secondBestPhotIndex1].eta();
      pho1Phi = _PFPhotons[secondBestPhotIndex1].phi();
      pho2Pt = _PFPhotons[secondBestPhotIndex2].pt();
      pho2Eta = _PFPhotons[secondBestPhotIndex2].eta();
      pho2Phi = _PFPhotons[secondBestPhotIndex2].phi();
      bjet1Pt = _PFPhotons[bestPhotIndex1].pt();
      bjet1Eta = _PFPhotons[bestPhotIndex1].eta();
      bjet1Phi = _PFPhotons[bestPhotIndex1].phi();
      bjet2Pt = _PFPhotons[bestPhotIndex2].pt();
      bjet2Eta = _PFPhotons[bestPhotIndex2].eta();
      bjet2Phi = _PFPhotons[bestPhotIndex2].phi();
    }      
    higgsPt = bestDiphoton.pt();
    if (higgsPt > 110){
      //cout << "higgsPt: "<<higgsPt<<endl;
      numBox = 0;
    }
    higgsEta = bestDiphoton.eta();
    higgsPhi = bestDiphoton.phi();
    higgsMass = bestDiphoton.m();

    // AK5 jets
    fastjet::JetDefinition AK05_def(fastjet::antikt_algorithm, 0.5);
    fastjet::ClusterSequence pfAK05ClusterSequence = JetMaker(JetsConst, AK05_def);
    vector<fastjet::PseudoJet> pfAK05 = SelectByAcceptance(fastjet::sorted_by_pt(pfAK05ClusterSequence.inclusive_jets()),30., 3.0); //only cluster jets with > 30 GeV, eta < 3.0
    std::cout << "--> pass ak5" << std::endl;
    if(pfAK05.size()<1){
        cout << "No jets..." << endl;
	       continue;
    }

    vector<fastjet::PseudoJet> jetsForHemispheres;
    fastjet::PseudoJet pho1(pho1Pt*cos(pho1Phi), pho1Pt*sin(pho1Phi), pho1Pt*sinh(pho1Eta), pho1Pt*cosh(pho1Eta));
    fastjet::PseudoJet pho2(pho2Pt*cos(pho2Phi), pho2Pt*sin(pho2Phi), pho2Pt*sinh(pho2Eta), pho2Pt*cosh(pho2Eta));
    fastjet::PseudoJet bjet1(bjet1Pt*cos(bjet1Phi), bjet1Pt*sin(bjet1Phi), bjet1Pt*sinh(bjet1Eta), bjet1Pt*cosh(bjet1Eta));
    fastjet::PseudoJet bjet2(bjet2Pt*cos(bjet2Phi), bjet2Pt*sin(bjet2Phi), bjet2Pt*sinh(bjet2Eta), bjet2Pt*cosh(bjet2Eta));
    fastjet::PseudoJet higgs = pho1 + pho2;
    double higgsPhi = higgs.phi();
    double higgsEta = higgs.eta();
    double higgsPt = higgs.pt();
    double higgsEnergy = higgs.E();
    TLorentzVector higgsvector;
    higgsvector.SetPtEtaPhiE(higgsPt, higgsEta, higgsPhi, higgsEnergy);
      
    jetsForHemispheres.push_back(higgs);
    std::cout << "--> pass hem" << std::endl;
    bool useHbb1 = false; //this says whether or not bjets are recognized
    bool useHbb2 = false;
    double thisDRB = -1;
    double thisDRB2 = -1;
    if (twoHiggs){
      cout << "found two Higgs, doing something weird" << endl;
      thisDRB = min(bjet1.delta_R(pho1), bjet1.delta_R(pho2));
      thisDRB2 = min(bjet2.delta_R(pho1), bjet2.delta_R(pho2));
      double bjetEff = ((double)rand()/(RAND_MAX)); 
      double bjetBR = ((double)rand()/(RAND_MAX));
      cout << "Random Number 2: " << bjetEff << endl;
      if (thisDRB > 0.5 && bjetEff < 0.6 && (bjetBR < 0.578)){ //this applies a rough medium b-jet efficiency cut
	  numBJets++;
	  useHbb1 = true;
      }
      bjetEff = ((double)rand()/(RAND_MAX));
      bjetBR = ((double)rand()/(RAND_MAX));	
      cout << "Random Number 3: " << bjetEff<< bjetBR << endl;
      if (thisDRB2 > 0.5 && bjetEff < 0.6 && (bjetBR < 0.578)){
	numBJets++;
	useHbb2 = true;
      }
    }

    double bJetEffList[pfAK05.size()];
    for(int i =0; i < pfAK05.size(); i++){
      bJetEffList[i] = ((double)rand()/(RAND_MAX));
    }

    for(int i = 0; i < pfAK05.size(); i++){
      //check if within DR < 0.5 of a selected photon
      double thisDR = min(pfAK05[i].delta_R(pho1), pfAK05[i].delta_R(pho2));
      if(thisDR < 0.5) continue;
      numJets++; //this includes the two photons that I set as fake b-jets, so should return correct number of jets
      //check if bjet
      if(IsBJet(pfAK05[i],0.5,30) && bJetEffList[i] < 0.6) {
	numBJets++;
	if (useHbb1){ //pair up the Hbbs with the other jets
	  double massTempB = (pfAK05[i] + bjet1).m();
	  if (numBox <0 && massTempB > 110 && massTempB < 140){
	    numBox = 1;
	  }
	}
	if (useHbb2){
	  double massTempB2 = (pfAK05[i] + bjet2).m();
	  if (numBox <0 && massTempB2 > 110 && massTempB2 < 140){
	    numBox = 1;
	  }
	}
	for(int j = 0; j < pfAK05.size(); j++){ //check if in Hbb box
	  double thisDR2 = min(pfAK05[j].delta_R(pho1), pfAK05[j].delta_R(pho2));
	  if (i==j || (thisDR2 < 0.5)) continue;
	  if(!IsBJet(pfAK05[j],0.5,30) && bJetEffList[j] > 0.6) continue;
	  double massTemp = (pfAK05[i] + pfAK05[j]).m();
	  if (numBox !=0 && massTemp > 110 && massTemp < 140){
	    numBox = 1;
	  }
	  /*if (numBox <0 && massTemp > 76 && massTemp < 106){
	    numBox = 2; //comment out later, this is for Zbb box
	    }*/
	}
      }
      jetPt[i] = pfAK05[i].pt();
      jetEta[i] = pfAK05[i].eta();
      jetPhi[i] = pfAK05[i].phi();
      jetsForHemispheres.push_back(pfAK05[i]);
    }

    if (firstPhot && useHbb1 && useHbb2){ 
      if (secondBestMass > 110 && secondBestMass < 140 && numBox<0){
	numBox = 1;
      }
    }
    else if (useHbb1 && useHbb2){
      if (bestMass > 110 && bestMass < 140 && numBox < 0){
	numBox = 1;
      }
    }

    if (numJets == 0){
      cout << "No jets found!!!!" << endl;
      continue;
    }
    GenMET();
    PFMET = genMET;
    MET = PFMET.pt();

    //traditional hemispheres
    if(jetsForHemispheres.size() < 2){
        cout << "Not enough objects for razor computation" << endl;
        continue;
    }
    std::cout << "--> before CMSHEM" << std::endl;
    CMSHemisphere* myHem = new CMSHemisphere(ConvertTo4Vector(jetsForHemispheres));
    myHem->CombineMinMass();
    vector<TLorentzVector> hem = myHem->GetHemispheres();
    vector<int> Temporary = myHem->GetHem1Constituents();
    vector<int> Temporary2 = myHem->GetHem2Constituents();
    std::cout << "--> pass CMSHEM" << std::endl;
    
    // SAVES HEM CONSTITUENTS
    int count1 = 0;
    int count2 = 0;
    for (int k=0; k < Temporary.size(); k++){
      if (Temporary[k]>-1) { 
	Hem1_csts[count1] = Temporary[k];
	count1++;
      }
    }
    for (int k=0; k < Temporary2.size(); k++){
      if (Temporary2[k]>-1) {
	Hem2_csts[count2] = Temporary2[k];
	count2++;
      }
    }
    int j = 0;
    std::cout << "pass SAVE" << std::endl;
    /*
    while (Temporary[j] > -1) {
      cout << "What'S going on: "<< Temporary[j] << endl;
      if (Temporary[j]==0 && count1==1) {
	// higgs only
	leadingHemContents = 0;
	break;
      }
      else if (Temporary[j]==0 && count1>1) {
	// higgs + jets
	leadingHemContents = 2;
	break;
      }
      else {
	leadingHemContents = 1;
      }
      j++;
    }
    */
    j = 0;
    std::cout << "pass SAVE 1" << std::endl;
    /*
    while (Temporary2[j] > -1){
      if (Temporary2[j]==0 && count2==1) {
	// higgs only
	cout << "entered this" << endl;
	subleadingHemContents = 0;
	break;
      }
      else if (Temporary2[j]==0 && count2>1) {
	// higgs + jets
	subleadingHemContents = 2;
	cout << "entered this" << endl;
	break;
      }
      else {
	subleadingHemContents = 1;
      }
      j++;
    }
    */
    std::cout << "before delete Hem" << std::endl;
    delete myHem;

    std::cout << "delete Hem" << std::endl;
    //compute traditional RSQ and MR
    TLorentzVector j1 = hem[0];
    TLorentzVector j2 = hem[1];  
    dPhi = DeltaPhi(j1,j2); //deltaPhi
    dPhiHiggsJ1 = DeltaPhi(higgsvector,j1);
    dPhiHiggsJ2 = DeltaPhi(higgsvector,j2);
    MR = CalcMR(j1, j2);
    RSQ = pow(CalcMRT(j1, j2, PFMET),2.)/MR/MR;
    cout << "numBox: "<<numBox<<endl;

    // write event in the tree
    outTree->Fill();

  }

  // full event TTree
  outTree->Write();
  
  file->cd();

  file->Close();


}

double CMSRazorHggHbb::DeltaPhi(TLorentzVector jet1, TLorentzVector jet2) {
  double deltaPhi = jet1.Phi() - jet2.Phi();
  while (deltaPhi > M_PI) deltaPhi -= 2*M_PI;
  while (deltaPhi <= -M_PI) deltaPhi += 2*M_PI;
  return deltaPhi;
}



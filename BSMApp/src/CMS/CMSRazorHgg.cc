#include <string>
#include <iostream>
#include <TTree.h>
#include "CMS/CMSRazorHgg.hh"
#include <fastjet/tools/Pruner.hh>

CMSRazorHgg::CMSRazorHgg(TTree *tree, double Lumi, string analysis) : CMSReco(tree) {
  _Lumi = Lumi;
  _statTools = new StatTools(-99);
  _analysis = analysis;
}

CMSRazorHgg::~CMSRazorHgg(){
}

void CMSRazorHgg::SetSqrts(double sqrts) {
  _sqrts = sqrts;
}

// loop over events - real analysis
void CMSRazorHgg::Loop(string outFileName) {

  if(fChain == 0) return;

  double MR, RSQ;

  double jetPt[50];
  double jetEta[50];
  double jetPhi[50];

  double pho1Pt, pho2Pt;
  double pho1Eta, pho2Eta;
  double pho1Phi, pho2Phi;

  double higgsPt, higgsEta, higgsPhi, higgsMass;
  int numJets;
  double MET;

  int genNumHiggs;

  //new variables
  int numBJets;
  int jetLen = 100;
  int Hem1_csts[jetLen], Hem2_csts[jetLen];
  double dPhi, dPhiHiggsJ1, dPhiHiggsJ2;
  int leadingHemContents, subleadingHemContents;

  // Open Output file
  TFile *file = new TFile(outFileName.c_str(),"UPDATE");

  TTree* outTree = new TTree("RazorInclusive","RazorInclusive");
  outTree->Branch("MR", &MR, "MR/D");
  outTree->Branch("RSQ", &RSQ, "RSQ/D");
  outTree->Branch("numJets", &numJets, "numJets/I");
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

  outTree->Branch("genNumHiggs", &genNumHiggs, "genNumHiggs/I");

  outTree->Branch("MET", &MET, "MET/D");

  // added branches
  outTree->Branch("dPhi", &dPhi, "dPhi/D");
  outTree->Branch("dPhiHiggsJ1", &dPhiHiggsJ1, "dPhiHiggsJ1/D");
  outTree->Branch("dPhiHiggsJ2", &dPhiHiggsJ2, "dPhiHiggsJ2/D");
  outTree->Branch("numBJets", &numBJets, "numBJets/I");
  outTree->Branch("leadingHemContents", &leadingHemContents, "leadingHemContents/I");
  outTree->Branch("subleadingHemContents", &subleadingHemContents, "subleadingHemContents/I"); //0 if higgs only, //1 if jets only, //2 if higgs and jets
  //outTree->Branch("MT", &MT, "MT/D");
  
  // loop over entries
  Long64_t nbytes = 0, nb = 0;
  Long64_t nentries = fChain->GetEntries();
  std::cout << "Number of entries = " << nentries << std::endl;

  // set the by-event weight
  for (Long64_t jentry=0; jentry<nentries;jentry+=1) {

    if(verbose) cout << "new event" << endl;

    // clean physics-objects blocks
    CleanEvent();

    // get new event
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    if (jentry%1000 == 0) std::cout << ">>> Processing event # " << jentry << std::endl;

    //reset jet variables
    numJets = 0;
    numBJets = 0;
    for(int i = 0; i < 50; i++){
        jetPt[i] = -1;
        jetEta[i] = -999;
        jetPhi[i] = -999;
    }
    pho1Pt = 0; pho1Eta = -999; pho1Phi = -999;
    pho2Pt = 0; pho2Eta = -999; pho2Phi = -999;
    higgsPt = 0; higgsEta = -999; higgsPhi = -999;
    higgsMass = -999;


    genNumHiggs = 0;
    for(int iGenTreeParticle = 0; iGenTreeParticle < GenTreeParticle; iGenTreeParticle++){
      if (GenTreeParticlePdgId[iGenTreeParticle]==25) genNumHiggs++;
    }


    // Build the event at generator level
    PFReco();
    vector<fastjet::PseudoJet> empty;
    vector<fastjet::PseudoJet> JetsConst = PFJetConstituents(empty,empty,empty);

    //Photons 
    double bestSumPt = -1;
    int bestPhotIndex1 = -1;
    int bestPhotIndex2 = -1;
    //loop over all pairs of gen photons and find the pair closest to Higgs mass
    for(int iPhot = 0; iPhot < _PFPhotons.size(); iPhot++){
        if(_PFPhotons[iPhot].pt() < 25 || abs(_PFPhotons[iPhot].eta()) > 1.48) continue;
        for(int jPhot = 0; jPhot < _PFPhotons.size(); jPhot++){
            if(iPhot == jPhot) continue;
            if(_PFPhotons[jPhot].pt() < 25 || abs(_PFPhotons[jPhot].eta()) > 1.48) continue;
            fastjet::PseudoJet higgsCandidate = _PFPhotons[iPhot] + _PFPhotons[jPhot];
            cout << higgsCandidate.pt() << " " << higgsCandidate.m() << endl;
            //check if this pair is in the correct mass range
            if(higgsCandidate.m() > 100 && higgsCandidate.m() < 180){
                cout << "Checking!" << endl;
                if(_PFPhotons[iPhot].pt() + _PFPhotons[jPhot].pt() > bestSumPt){
                    cout << "Made it!" << endl;
                        bestSumPt = _PFPhotons[iPhot].pt() + _PFPhotons[jPhot].pt();
                        bestPhotIndex1 = iPhot;
                        bestPhotIndex2 = jPhot;
                }
            }
        }
    }
    if(bestPhotIndex1 == -1){
        cout << "Didn't find a higgs candidate" << endl;
        continue; ///continue if we didn't find a higgs candidate
    }
    cout << "Found higgs candidate!!!!" << endl;
    fastjet::PseudoJet bestDiphoton = _PFPhotons[bestPhotIndex1] + _PFPhotons[bestPhotIndex2];
    if(bestDiphoton.pt() < 20){
        cout << "Diphoton system has pT too small" << endl; 
        continue;
    }
    //fill photon variables
    pho1Pt = _PFPhotons[bestPhotIndex1].pt();
    pho1Eta = _PFPhotons[bestPhotIndex1].eta();
    pho1Phi = _PFPhotons[bestPhotIndex1].phi();
    pho2Pt = _PFPhotons[bestPhotIndex2].pt();
    pho2Eta = _PFPhotons[bestPhotIndex2].eta();
    pho2Phi = _PFPhotons[bestPhotIndex2].phi();
    higgsPt = bestDiphoton.pt();
    higgsEta = bestDiphoton.eta();
    higgsPhi = bestDiphoton.phi();
    higgsMass = bestDiphoton.m();

    // AK5 jets
    fastjet::JetDefinition AK05_def(fastjet::antikt_algorithm, 0.5);
    fastjet::ClusterSequence pfAK05ClusterSequence = JetMaker(JetsConst, AK05_def);
    vector<fastjet::PseudoJet> pfAK05 = SelectByAcceptance(fastjet::sorted_by_pt(pfAK05ClusterSequence.inclusive_jets()),30., 3.0);

    if(pfAK05.size()<1){
        cout << "No jets..." << endl;
        continue;
    }

    vector<fastjet::PseudoJet> jetsForHemispheres;
    fastjet::PseudoJet pho1(pho1Pt*cos(pho1Phi), pho1Pt*sin(pho1Phi), pho1Pt*sinh(pho1Eta), pho1Pt*cosh(pho1Eta));
    fastjet::PseudoJet pho2(pho2Pt*cos(pho2Phi), pho2Pt*sin(pho2Phi), pho2Pt*sinh(pho2Eta), pho2Pt*cosh(pho2Eta));
    fastjet::PseudoJet higgs = pho1 + pho2;
    double higgsPhi = higgs.phi();
    double higgsEta = higgs.eta();
    double higgsPt = higgs.pt();
    double higgsEnergy = higgs.E();
    TLorentzVector higgsvector;
    higgsvector.SetPtEtaPhiE(higgsPt, higgsEta, higgsPhi, higgsEnergy);
      
    jetsForHemispheres.push_back(higgs);
    for(int i = 0; i < pfAK05.size(); i++){
        //check if within DR < 0.5 of a selected photon
        double thisDR = min(pfAK05[i].delta_R(pho1), pfAK05[i].delta_R(pho2));
        if(thisDR < 0.5) continue;
	//check if bjet
	if(IsBJet(pfAK05[i],0.5,30)) numBJets++;
        numJets++;
        jetPt[i] = pfAK05[i].pt();
        jetEta[i] = pfAK05[i].eta();
        jetPhi[i] = pfAK05[i].phi();
        jetsForHemispheres.push_back(pfAK05[i]);
    }

    GenMET();
    PFMET = genMET;
    MET = PFMET.pt();

    //traditional hemispheres
    if(jetsForHemispheres.size() < 2){
        cout << "Not enough objects for razor computation" << endl;
        continue;
    }
    
    CMSHemisphere* myHem = new CMSHemisphere(ConvertTo4Vector(jetsForHemispheres));
    myHem->CombineMinMass();
    vector<TLorentzVector> hem = myHem->GetHemispheres();
    vector<int> Temporary = myHem->GetHem1Constituents();
    vector<int> Temporary2 = myHem->GetHem2Constituents();
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
    j = 0;
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

    delete myHem;
    //compute traditional RSQ and MR
    TLorentzVector j1 = hem[0];
    TLorentzVector j2 = hem[1];  
    dPhi = DeltaPhi(j1,j2); //deltaPhi
    dPhiHiggsJ1 = DeltaPhi(higgsvector,j1);
    dPhiHiggsJ2 = DeltaPhi(higgsvector,j2);
    MR = CalcMR(j1, j2);
    RSQ = pow(CalcMRT(j1, j2, PFMET),2.)/MR/MR;
    
    // write event in the tree
    outTree->Fill();

  }

  // full event TTree
  outTree->Write();
  
  file->cd();

  file->Close();

}

double CMSRazorHgg::DeltaPhi(TLorentzVector jet1, TLorentzVector jet2) {
  double deltaPhi = jet1.Phi() - jet2.Phi();
  while (deltaPhi > M_PI) deltaPhi -= 2*M_PI;
  while (deltaPhi <= -M_PI) deltaPhi += 2*M_PI;
  return deltaPhi;
}



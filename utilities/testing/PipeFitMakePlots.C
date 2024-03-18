void PipeFitMakePlots(Int_t nzip){
  //the histograms
  TH1F *hAa0 = new TH1F("hAa0","",200,0,1000);
  TH1F *hAa1 = new TH1F("hAa1","",30,-1,2);
  TH1F *hAt0fit = new TH1F("hAt0fit","",100,300,600);
  TH1F *hAtau = new TH1F("hAtau","",50,0,500);
  TH1F *hAkappa = new TH1F("hAkappa","",50,0,500);

  

  TH1F *hBa0 = new TH1F("hBa0","",200,0,1000);
  TH1F *hBa1 = new TH1F("hBa1","",30,-1,2);
  TH1F *hBt0fit = new TH1F("hBt0fit","",100,300,600);
  TH1F *hBtau = new TH1F("hBtau","",50,0,500);
  TH1F *hBkappa = new TH1F("hBkappa","",50,0,500);

  TH1F *hCa0 = new TH1F("hCa0","",200,0,1000);
  TH1F *hCa1 = new TH1F("hCa1","",30,-1,2);
  TH1F *hCt0fit = new TH1F("hCt0fit","",100,300,600);
  TH1F *hCtau = new TH1F("hCtau","",50,0,500);
  TH1F *hCkappa = new TH1F("hCkappa","",50,0,500);

  TH1F *hDa0 = new TH1F("hDa0","",200,0,1000);
  TH1F *hDa1 = new TH1F("hDa1","",30,-1,2);
  TH1F *hDt0fit = new TH1F("hDt0fit","",100,300,600);
  TH1F *hDtau = new TH1F("hDtau","",50,0,500);
  TH1F *hDkappa = new TH1F("hDkappa","",50,0,500);
 

  //fall function histograms
 
  TH1F *hAaf = new TH1F("hAaf","",10000,0,500000);
  TH1F *hAtf1 = new TH1F("hAtf1","",100,0,2000);
  TH1F *hAtf2 = new TH1F("hAtf2","",100,0,2000);
  TH1F *hAtfr = new TH1F("hAtfr","",100,-1,1);

  TH1F *hBaf = new TH1F("hBaf","",10000,0,500000);
  TH1F *hBtf1 = new TH1F("hBtf1","",100,0,2000);
  TH1F *hBtf2 = new TH1F("hBtf2","",100,0,2000);
  TH1F *hBtfr = new TH1F("hBtfr","",100,-1,1);

  TH1F *hCaf = new TH1F("hCaf","",10000,0,500000);
  TH1F *hCtf1 = new TH1F("hCtf1","",100,0,2000);
  TH1F *hCtf2 = new TH1F("hCtf2","",100,0,2000);
  TH1F *hCtfr = new TH1F("hCtfr","",100,-1,1);

  TH1F *hDaf = new TH1F("hDaf","",10000,0,500000);
  TH1F *hDtf1 = new TH1F("hDtf1","",100,0,2000);
  TH1F *hDtf2 = new TH1F("hDtf2","",100,0,2000);
  TH1F *hDtfr = new TH1F("hDtfr","",100,-1,1);

  Float_t Aa0,Aa1,Aaf,At0fit,Atau,Akappa,Atf1,Atf2,Atfr,Arfdof,Arfchi2,Affdof,Affchi2;
  Float_t Ba0,Ba1,Baf,Bt0fit,Btau,Bkappa,Btf1,Btf2,Btfr,Brfdof,Brfchi2,Bffdof,Bffchi2;
  Float_t Ca0,Ca1,Caf,Ct0fit,Ctau,Ckappa,Ctf1,Ctf2,Ctfr,Crfdof,Crfchi2,Cffdof,Cffchi2;
  Float_t Da0,Da1,Daf,Dt0fit,Dtau,Dkappa,Dtf1,Dtf2,Dtfr,Drfdof,Drfchi2,Dffdof,Dffchi2;

 Int_t i;
 char file_name[40];
 sprintf(file_name,"output_tree_zip%d.root",nzip);
 TFile *F = new TFile(file_name, "read");
 char tree_name[20];
 sprintf(tree_name,"outzip%d",nzip);
 TTree *T = (TTree *)F->Get(tree_name);
 
 T->SetBranchAddress("Aa0",&Aa0);
 T->SetBranchAddress("Aa1",&Aa1);
 T->SetBranchAddress("At0fit",&At0fit);
 T->SetBranchAddress("Atau",&Atau);
 T->SetBranchAddress("Akappa",&Akappa);
 T->SetBranchAddress("Aaf",&Aaf);
 T->SetBranchAddress("Atf1",&Atf1);
 T->SetBranchAddress("Atf2",&Atf2);
 T->SetBranchAddress("Atfr",&Atfr);
 T->SetBranchAddress("Arfchi2",&Arfchi2);
 T->SetBranchAddress("Arfdof",&Arfdof);
 T->SetBranchAddress("Affchi2",&Affchi2);
 T->SetBranchAddress("Affdof",&Affdof);
    
    //channel B
 T->SetBranchAddress("Ba0",&Ba0);
 T->SetBranchAddress("Ba1",&Ba1);
 T->SetBranchAddress("Bt0fit",&Bt0fit);
 T->SetBranchAddress("Btau",&Btau);
 T->SetBranchAddress("Bkappa",&Bkappa);
 T->SetBranchAddress("Baf",&Baf);
 T->SetBranchAddress("Btf1",&Btf1);
 T->SetBranchAddress("Btf1",&Btf2);
 T->SetBranchAddress("Btfr",&Btfr);
 T->SetBranchAddress("Brfchi2",&Brfchi2);
 T->SetBranchAddress("Brfdof",&Brfdof);
 T->SetBranchAddress("Bffchi2",&Bffchi2);
 T->SetBranchAddress("Bffdof",&Bffdof);

    //channel C
 T->SetBranchAddress("Ca0",&Ca0);
 T->SetBranchAddress("Ca1",&Ca1);
 T->SetBranchAddress("Ct0fit",&Ct0fit);
 T->SetBranchAddress("Ctau",&Ctau);
 T->SetBranchAddress("Ckappa",&Ckappa);
 T->SetBranchAddress("Caf",&Caf);
 T->SetBranchAddress("Ctf1",&Ctf1);
 T->SetBranchAddress("Ctf2",&Ctf2);
 T->SetBranchAddress("Ctfr",&Ctfr);
 T->SetBranchAddress("Crfchi2",&Crfchi2);
 T->SetBranchAddress("Crfdof",&Crfdof);
 T->SetBranchAddress("Cffchi2",&Cffchi2);
 T->SetBranchAddress("Cffdof",&Cffdof);

    //channel D
 T->SetBranchAddress("Da0",&Da0);
 T->SetBranchAddress("Da1",&Da1);
 T->SetBranchAddress("Dt0fit",&Dt0fit);
 T->SetBranchAddress("Dtau",&Dtau);
 T->SetBranchAddress("Dkappa",&Dkappa);
 T->SetBranchAddress("Daf",&Daf);
 T->SetBranchAddress("Dtf1",&Dtf1);
 T->SetBranchAddress("Dtf2",&Dtf2);
 T->SetBranchAddress("Dtfr",&Dtfr);
 T->SetBranchAddress("Drfchi2",&Drfchi2);
 T->SetBranchAddress("Drfdof",&Drfdof);
 T->SetBranchAddress("Dffchi2",&Dffchi2);
 T->SetBranchAddress("Dffdof",&Dffdof); 

 Int_t nentries = T->GetEntries();
 printf("Number of events %d\n",nentries);

 TH1F *hAa0n = new TH1F("hAa0n","",nentries,0,nentries);
 TH1F *hAa1n = new TH1F("hAa1n","",nentries,0,nentries);
 TH1F *hAt0fitn = new TH1F("hAt0fitn","",nentries,0,nentries);
 TH1F *hAkappan = new TH1F("hAkappan","",nentries,0,nentries);
 TH1F *hAtaun = new TH1F("hAtaun","",nentries,0,nentries);

 TH1F *hBa0n = new TH1F("hBa0n","",nentries,0,nentries);
 TH1F *hBa1n = new TH1F("hBa1n","",nentries,0,nentries);
 TH1F *hBt0fitn = new TH1F("hBt0fitn","",nentries,0,nentries);
 TH1F *hBkappan = new TH1F("hBkappan","",nentries,0,nentries);
 TH1F *hBtaun = new TH1F("hBtaun","",nentries,0,nentries);

 TH1F *hCa0n = new TH1F("hCa0n","",nentries,0,nentries);
 TH1F *hCa1n = new TH1F("hCa1n","",nentries,0,nentries);
 TH1F *hCt0fitn = new TH1F("hCt0fitn","",nentries,0,nentries);
 TH1F *hCkappan = new TH1F("hCkappan","",nentries,0,nentries);
 TH1F *hCtaun = new TH1F("hCtaun","",nentries,0,nentries);

 TH1F *hDa0n = new TH1F("hDa0n","",nentries,0,nentries);
 TH1F *hDa1n = new TH1F("hDa1n","",nentries,0,nentries);
 TH1F *hDt0fitn = new TH1F("hDt0fitn","",nentries,0,nentries);
 TH1F *hDkappan = new TH1F("hDkappan","",nentries,0,nentries);
 TH1F *hDtaun = new TH1F("hDtaun","",nentries,0,nentries);

 TH1F *hAafn = new TH1F("hAafn","",nentries,0,nentries);
 TH1F *hAtf1n = new TH1F("hAtf1n","",nentries,0,nentries);
 TH1F *hAtf2n = new TH1F("hAtf2n","",nentries,0,nentries);
 TH1F *hAtfrn = new TH1F("hAtfrn","",nentries,0,nentries);

 TH1F *hBafn = new TH1F("hBafn","",nentries,0,nentries);
 TH1F *hBtf1n = new TH1F("hBtf1n","",nentries,0,nentries);
 TH1F *hBtf2n = new TH1F("hBtf2n","",nentries,0,nentries);
 TH1F *hBtfrn = new TH1F("hBtfrn","",nentries,0,nentries);

 TH1F *hCafn = new TH1F("hCafn","",nentries,0,nentries);
 TH1F *hCtf1n = new TH1F("hCtf1n","",nentries,0,nentries);
 TH1F *hCtf2n = new TH1F("hCtf2n","",nentries,0,nentries);
 TH1F *hCtfrn = new TH1F("hCtfrn","",nentries,0,nentries);

 TH1F *hDafn = new TH1F("hDafn","",nentries,0,nentries);
 TH1F *hDtf1n = new TH1F("hDtf1n","",nentries,0,nentries);
 TH1F *hDtf2n = new TH1F("hDtf2n","",nentries,0,nentries);
 TH1F *hDtfrn = new TH1F("hDtfrn","",nentries,0,nentries);

 Int_t i;
  
 for(i=0;i<nentries;i++){
   T->GetEntry(i+1);
   
   hAa0->Fill(Aa0);
   hAa1->Fill(Aa1);
   hAt0fit->Fill(At0fit);
   hAkappa->Fill(Akappa);
   hAtau->Fill(Atau);

   hBa0->Fill(Ba0);
   hBa1->Fill(Ba1);
   hBt0fit->Fill(Bt0fit);
   hBkappa->Fill(Bkappa);
   hBtau->Fill(Btau);

   hCa0->Fill(Ca0);
   hCa1->Fill(Ca1);
   hCt0fit->Fill(Ct0fit);
   hCkappa->Fill(Ckappa);
   hCtau->Fill(Ctau);

   hDa0->Fill(Da0);
   hDa1->Fill(Da1);
   hDt0fit->Fill(Dt0fit);
   hDkappa->Fill(Dkappa);
   hDtau->Fill(Dtau);

   if (Aa0>0) hAa0n->SetBinContent(i+1,Aa0);
   if (Aa1>0) hAa1n->SetBinContent(i+1,Aa1);
   if (At0fit>0) hAt0fitn->SetBinContent(i+1,At0fit);
   if (Akappa>0) hAkappan->SetBinContent(i+1,Akappa);
   if (Atau>0) hAtaun->SetBinContent(i+1,Atau);

   if (Ba0>0) hBa0n->SetBinContent(i+1,Ba0);
   if (Ba1>0) hBa1n->SetBinContent(i+1,Ba1);
   if (Bt0fit>0) hBt0fitn->SetBinContent(i+1,Bt0fit);
   if (Bkappa>0) hBkappan->SetBinContent(i+1,Bkappa);
   if (Btau>0) hBtaun->SetBinContent(i+1,Btau);

   if (Ca0>0) hCa0n->SetBinContent(i+1,Ca0);
   if (Ca1>0) hCa1n->SetBinContent(i+1,Ca1);
   if (Ct0fit>0) hCt0fitn->SetBinContent(i+1,Ct0fit);
   if (Ckappa>0) hCkappan->SetBinContent(i+1,Ckappa);
   if (Ctau>0) hCtaun->SetBinContent(i+1,Ctau);

   if (Da0>0) hDa0n->SetBinContent(i+1,Da0);
   if (Da1>0) hDa1n->SetBinContent(i+1,Da1);
   if (Dt0fit>0) hDt0fitn->SetBinContent(i+1,Dt0fit);
   if (Dkappa>0) hDkappan->SetBinContent(i+1,Dkappa);
   if (Dtau>0) hDtaun->SetBinContent(i+1,Dtau);

   //Fall function parameters
   hAaf->Fill(Aaf);
   hAtf1->Fill(Atf1);
   hAtf2->Fill(Atf2);
   hAtfr->Fill(Atfr);

   hBaf->Fill(Baf);
   hBtf1->Fill(Btf1);
   hBtf2->Fill(Btf2);
   hBtfr->Fill(Btfr);

   hCaf->Fill(Caf);
   hCtf1->Fill(Ctf1);
   hCtf2->Fill(Ctf2);
   hCtfr->Fill(Ctfr);

   hDaf->Fill(Daf);
   hDtf1->Fill(Dtf1);
   hDtf2->Fill(Dtf2);
   hDtfr->Fill(Dtfr);

   if (Aaf>0) hAafn->SetBinContent(i+1,Aaf);
   if (Atf1>0) hAtf1n->SetBinContent(i+1,Atf1);
   if (Atf2>0) hAtf2n->SetBinContent(i+1,Atf2);
   if (Atfr>0) hAtfrn->SetBinContent(i+1,Atfr);

   if (Baf>0) hBafn->SetBinContent(i+1,Baf);
   if (Btf1>0) hBtf1n->SetBinContent(i+1,Btf1);
   if (Btf2>0) hBtf2n->SetBinContent(i+1,Btf2);
   if (Btfr>0) hBtfrn->SetBinContent(i+1,Btfr);

   if (Caf>0) hCafn->SetBinContent(i+1,Caf);
   if (Ctf1>0) hCtf1n->SetBinContent(i+1,Ctf1);
   if (Ctf2>0) hCtf2n->SetBinContent(i+1,Ctf2);
   if (Ctfr>0) hCtfrn->SetBinContent(i+1,Ctfr);

   if (Daf>0) hDafn->SetBinContent(i+1,Daf);
   if (Dtf1>0) hDtf1n->SetBinContent(i+1,Dtf1);
   if (Dtf2>0) hDtf2n->SetBinContent(i+1,Dtf2);
   if (Dtfr>0) hDtfrn->SetBinContent(i+1,Dtfr);
   

   printf("i=%d   A0 %f B0 %f  C0  %f D0 %f\n",i+1,Aa0,Ba0,Ca0,Da0);

 }

 char plot_file[40];
 sprintf(plot_file,"PFPplots_zip%d.root",nzip);
 TFile *fr = new TFile(plot_file,"RECREATE");
 hAa0->Write();
 hAa1->Write();
 hAt0fit->Write();
 hAkappa->Write();
 hAtau->Write();
 
 hBa0->Write();
 hBa1->Write();
 hBt0fit->Write();
 hBkappa->Write();
 hBtau->Write();

 hCa0->Write();
 hCa1->Write();
 hCt0fit->Write();
 hCkappa->Write();
 hCtau->Write();

 hDa0->Write();
 hDa1->Write();
 hDt0fit->Write();
 hDkappa->Write();
 hDtau->Write();
   
 hAaf->Write();
 hAtf1->Write();
 hAtf2->Write();
 hAtfr->Write();

 hBaf->Write();
 hBtf1->Write();
 hBtf2->Write();
 hBtfr->Write();

 hCaf->Write();
 hCtf1->Write();
 hCtf2->Write();
 hCtfr->Write();

 hDaf->Write();
 hDtf1->Write();
 hDtf2->Write();
 hDtfr->Write();

 //=======================================

 hAa0n->Write();
 hAa1n->Write();
 hAt0fitn->Write();
 hAkappan->Write();
 hAtaun->Write();
 
 hBa0n->Write();
 hBa1n->Write();
 hBt0fitn->Write();
 hBkappan->Write();
 hBtaun->Write();

 hCa0n->Write();
 hCa1n->Write();
 hCt0fitn->Write();
 hCkappan->Write();
 hCtaun->Write();

 hDa0n->Write();
 hDa1n->Write();
 hDt0fitn->Write();
 hDkappan->Write();
 hDtaun->Write();
   
 hAafn->Write();
 hAtf1n->Write();
 hAtf2n->Write();
 hAtfrn->Write();

 hBafn->Write();
 hBtf1n->Write();
 hBtf2n->Write();
 hBtfrn->Write();

 hCafn->Write();
 hCtf1n->Write();
 hCtf2n->Write();
 hCtfrn->Write();

 hDafn->Write();
 hDtf1n->Write();
 hDtf2n->Write();
 hDtfrn->Write();
}

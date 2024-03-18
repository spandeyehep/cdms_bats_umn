//Generate a root file with only the Pipefitphonon RQ's
//Input is the zip number

void PipeFitTest2(Int_t nzip){
 Int_t i;
 TFile *F = new TFile("test_170319_1616_F0002.gz.root", "read");
 char tree_name[20];
 sprintf(tree_name,"zip%d",nzip);
 Double_t PAa0,PAa1,PAaf,PAt0fit,PAtau,PAkappa,PAtf1,PAtf2,PAtfr,PArfdof,PArfchi2,PAffdof,PAffchi2;
 Double_t PBa0,PBa1,PBaf,PBt0fit,PBtau,PBkappa,PBtf1,PBtf2,PBtfr,PBrfdof,PBrfchi2,PBffdof,PBffchi2;
 Double_t PCa0,PCa1,PCaf,PCt0fit,PCtau,PCkappa,PCtf1,PCtf2,PCtfr,PCrfdof,PCrfchi2,PCffdof,PCffchi2;
 Double_t PDa0,PDa1,PDaf,PDt0fit,PDtau,PDkappa,PDtf1,PDtf2,PDtfr,PDrfdof,PDrfchi2,PDffdof,PDffchi2;
 TTree *T = (TTree *)F->Get(tree_name);
  
 //Channel A

    T->SetBranchAddress("PAa0",&PAa0);
    T->SetBranchAddress("PAa1",&PAa1);
    T->SetBranchAddress("PAt0fit",&PAt0fit);
    T->SetBranchAddress("PAtau",&PAtau);
    T->SetBranchAddress("PAkappa",&PAkappa);
    T->SetBranchAddress("PAaf",&PAaf);
    T->SetBranchAddress("PAtf1",&PAtf1);
    T->SetBranchAddress("PAtf2",&PAtf2);
    T->SetBranchAddress("PAtfr",&PAtfr);
    T->SetBranchAddress("PArfchi2",&PArfchi2);
    T->SetBranchAddress("PArfdof",&PArfdof);
    T->SetBranchAddress("PAffchi2",&PAffchi2);
    T->SetBranchAddress("PAffdof",&PAffdof);
    
    //channel B
    T->SetBranchAddress("PBa0",&PBa0);
    T->SetBranchAddress("PBa1",&PBa1);
    T->SetBranchAddress("PBt0fit",&PBt0fit);
    T->SetBranchAddress("PBtau",&PBtau);
    T->SetBranchAddress("PBkappa",&PBkappa);
    T->SetBranchAddress("PBaf",&PBaf);
    T->SetBranchAddress("PBtf1",&PBtf1);
    T->SetBranchAddress("PBtf1",&PBtf2);
    T->SetBranchAddress("PBtfr",&PBtfr);
    T->SetBranchAddress("PBrfchi2",&PBrfchi2);
    T->SetBranchAddress("PBrfdof",&PBrfdof);
    T->SetBranchAddress("PBffchi2",&PBffchi2);
    T->SetBranchAddress("PBffdof",&PBffdof);

    //channel C
    T->SetBranchAddress("PCa0",&PCa0);
    T->SetBranchAddress("PCa1",&PCa1);
    T->SetBranchAddress("PCt0fit",&PCt0fit);
    T->SetBranchAddress("PCtau",&PCtau);
    T->SetBranchAddress("PCkappa",&PCkappa);
    T->SetBranchAddress("PCaf",&PCaf);
    T->SetBranchAddress("PCtf1",&PCtf1);
    T->SetBranchAddress("PCtf2",&PCtf2);
    T->SetBranchAddress("PCtfr",&PCtfr);
    T->SetBranchAddress("PCrfchi2",&PCrfchi2);
    T->SetBranchAddress("PCrfdof",&PCrfdof);
    T->SetBranchAddress("PCffchi2",&PCffchi2);
    T->SetBranchAddress("PCffdof",&PCffdof);

    //channel D
    T->SetBranchAddress("PDa0",&PDa0);
    T->SetBranchAddress("PDa1",&PDa1);
    T->SetBranchAddress("PDt0fit",&PDt0fit);
    T->SetBranchAddress("PDtau",&PDtau);
    T->SetBranchAddress("PDkappa",&PDkappa);
    T->SetBranchAddress("PDaf",&PDaf);
    T->SetBranchAddress("PDtf1",&PDtf1);
    T->SetBranchAddress("PDtf2",&PDtf2);
    T->SetBranchAddress("PDtfr",&PDtfr);
    T->SetBranchAddress("PDrfchi2",&PDrfchi2);
    T->SetBranchAddress("PDrfdof",&PDrfdof);
    T->SetBranchAddress("PDffchi2",&PDffchi2);
    T->SetBranchAddress("PDffdof",&PDffdof);

 char out_file_name[40];
 char out_tree_name[40];
 sprintf(out_file_name,"output_tree_zip%d.root",nzip);
 sprintf(out_tree_name,"outzip%d",nzip); 
 TFile outfile(out_file_name,"RECREATE");
 

 Float_t Aa0,Aa1,Aaf,At0fit,Atau,Akappa,Atf1,Atf2,Atfr,Arfdof,Arfchi2,Affdof,Affchi2;
 Float_t Ba0,Ba1,Baf,Bt0fit,Btau,Bkappa,Btf1,Btf2,Btfr,Brfdof,Brfchi2,Bffdof,Bffchi2;
 Float_t Ca0,Ca1,Caf,Ct0fit,Ctau,Ckappa,Ctf1,Ctf2,Ctfr,Crfdof,Crfchi2,Cffdof,Cffchi2;
 Float_t Da0,Da1,Daf,Dt0fit,Dtau,Dkappa,Dtf1,Dtf2,Dtfr,Drfdof,Drfchi2,Dffdof,Dffchi2;
 
 TTree *tout = new TTree(out_tree_name,"Skimmed tree");

    

 tout->Branch("Aa0",&Aa0,"Aa0/F");
 tout->Branch("Aa1",&Aa1,"Aa1/F");
 tout->Branch("At0fit",&At0fit,"At0fit/F");
 tout->Branch("Atau",&Atau,"Atau/F");
 tout->Branch("Akappa",&Akappa,"Akappa/F");
 tout->Branch("Aa1",&Aa1,"Aa1/F");
 tout->Branch("Aaf",&Aaf,"Aaf/F");
 tout->Branch("Atf1",&Atf1,"Atf1/F");
 tout->Branch("Atf2",&Atf2,"Atf2/F");
 tout->Branch("Atfr",&Atfr,"Atfr/F");
 tout->Branch("Arfchi2",&Arfchi2,"Arfchi2/F");
 tout->Branch("Arfdof",&Arfdof,"Arfdof/F");
 tout->Branch("Affchi2",&Affchi2,"Affchi2/F");
 tout->Branch("Affdof",&Affdof,"Affdof/F");

 //Channel B
 tout->Branch("Ba0",&Ba0,"Ba0/F");
 tout->Branch("Ba1",&Ba1,"Ba1/F");
 tout->Branch("Bt0fit",&Bt0fit,"Bt0fit/F");
 tout->Branch("Btau",&Btau,"Btau/F");
 tout->Branch("Bkappa",&Bkappa,"Bkappa/F");
 tout->Branch("Ba1",&Ba1,"Ba1/F");
 tout->Branch("Baf",&Baf,"Baf/F");
 tout->Branch("Btf1",&Btf1,"Btf1/F");
 tout->Branch("Btf2",&Btf2,"Btf2/F");
 tout->Branch("Btfr",&Btfr,"Btfr/F");
 tout->Branch("Brfchi2",&Brfchi2,"Brfchi2/F");
 tout->Branch("Brfdof",&Brfdof,"Brfdof/F");
 tout->Branch("Bffchi2",&Bffchi2,"Bffchi2/F");
 tout->Branch("Bffdof",&Bffdof,"Bffdof/F");

 //Channel C
 tout->Branch("Ca0",&Ca0,"Ca0/F");
 tout->Branch("Ca1",&Ca1,"Ca1/F");
 tout->Branch("Ct0fit",&Ct0fit,"Ct0fit/F");
 tout->Branch("Ctau",&Ctau,"Ctau/F");
 tout->Branch("Ckappa",&Ckappa,"Ckappa/F");
 tout->Branch("Ca1",&Ca1,"Ca1/F");
 tout->Branch("Caf",&Caf,"Caf/F");
 tout->Branch("Ctf1",&Ctf1,"Ctf1/F");
 tout->Branch("Ctf2",&Ctf2,"Ctf2/F");
 tout->Branch("Ctfr",&Ctfr,"Ctfr/F");
 tout->Branch("Crfchi2",&Crfchi2,"Crfchi2/F");
 tout->Branch("Crfdof",&Crfdof,"Crfdof/F");
 tout->Branch("Cffchi2",&Cffchi2,"Cffchi2/F");
 tout->Branch("Cffdof",&Cffdof,"Cffdof/F");
  
 //Channel D
 tout->Branch("Da0",&Da0,"Da0/F");
 tout->Branch("Da1",&Da1,"Da1/F");
 tout->Branch("Dt0fit",&Dt0fit,"Dt0fit/F");
 tout->Branch("Dtau",&Dtau,"Dtau/F");
 tout->Branch("Dkappa",&Dkappa,"Dkappa/F");
 tout->Branch("Da1",&Da1,"Da1/F");
 tout->Branch("Daf",&Daf,"Daf/F");
 tout->Branch("Dtf1",&Dtf1,"Dtf1/F");
 tout->Branch("Dtf2",&Dtf2,"Dtf2/F");
 tout->Branch("Dtfr",&Dtfr,"Dtfr/F");
 tout->Branch("Drfchi2",&Drfchi2,"Drfchi2/F");
 tout->Branch("Drfdof",&Drfdof,"Drfdof/F");
 tout->Branch("Dffchi2",&Dffchi2,"Dffchi2/F");
 tout->Branch("Dffdof",&Dffdof,"Dffdof/F");

    Int_t nentries = T->GetEntries();
    printf("Number of events %d\n",nentries);
    for(i=0;i<nentries;i++){
      T->GetEntry(i+1);
      Aa0=PAa0;
      Aa1=PAa1;
      At0fit=PAt0fit;
      Atau=PAtau;
      Akappa=PAkappa;

      Aaf=PAaf; 
      Atf1=PAtf1;
      Atf2=PAtf2;
      Atfr=PAtfr;
          

      Arfchi2=PArfchi2;
      Arfdof=PArfdof;
      Affchi2=PAffchi2;
      Affdof=PAffdof;

      
      
      Ba0=PBa0;
      Ba1=PBa1;
      Bt0fit=PBt0fit;
      Btau=PBtau;
      Bkappa=PBkappa;

      Baf=PBaf; 
      Btf1=PBtf1;
      Btf2=PBtf2;
      Btfr=PBtfr;

      Brfchi2=PBrfchi2;
      Brfdof=PBrfdof;
      Bffchi2=PBffchi2;
      Bffdof=PBffdof;

      

      Ca0=PCa0;
      Ca1=PCa1;
      Ct0fit=PCt0fit;
      Ctau=PCtau;
      Ckappa=PCkappa;

      Caf=PCaf; 
      Ctf1=PCtf1;
      Ctf2=PCtf2;
      Ctfr=PCtfr;

      Crfchi2=PCrfchi2;
      Crfdof=PCrfdof;
      Cffchi2=PCffchi2;
      Cffdof=PCffdof;

      Da0=PDa0;
      Da1=PDa1;
      Dt0fit=PDt0fit;
      Dtau=PDtau;
      Dkappa=PDkappa;

      Daf=PDaf; 
      Dtf1=PDtf1;
      Dtf2=PDtf2;
      Dtfr=PDtfr;

      Drfchi2=PDrfchi2;
      Drfdof=PDrfdof;
      Dffchi2=PDffchi2;
      Dffdof=PDffdof;

      printf("i=%d   A0 %f B0 %f  C0  %f D0 %f\n",i+1,Aa0,Ba0,Ca0,Da0);
      
      tout->Fill();
    }
  
  outfile.cd();
  tout->Write();
  outfile.Close();
}

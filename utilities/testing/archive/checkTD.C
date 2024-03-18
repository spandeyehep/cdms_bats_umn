void constructFit(int pulseSize, float* para, TF1* func1, TF1* func2, float midpoint);
void constructFit();

static const int nBins = 2048; //for Soudan only

void checkTD(TString filename="testhisto.root")
{
    Int_t maxEvents = 100;
    Int_t evtCtr = 0;

    TFile f(filename);
    TTree* myTree = f.Get("pulseTree");

    //Variables
    Float_t pulseBSN[nBins]; 
    Int_t series;
    Int_t event;
    Int_t detectorNum;
    Int_t channelNum;
    Int_t isSmall;
    Int_t isMedium;
    Int_t isLarge;
    Float_t para[6];
    Float_t midpoint;
    Int_t pulseSize = -1;

    //Fit Functions
    TF1* func1 = new TF1("ftime_domain","[0]*(1-exp(-(x-[2])/[1]))*(exp(-(x-[2])/[3])-[4]*exp(-(x-[2])/[1]))", 0.0, (double)nBins);
    TF1* func2 = new TF1("ftime_domain2","[0]*(exp(-x/[1])+[2]*exp(-x/[3]))", 0.0, (double)nBins);

    //Branch Addresses
    myTree->SetBranchAddress("series", &series);
    myTree->SetBranchAddress("event", &event);
    myTree->SetBranchAddress("detectorNum", &detectorNum);
    myTree->SetBranchAddress("channelNum", &channelNum);
    myTree->SetBranchAddress("bsnPulse", pulseBSN);
    myTree->SetBranchAddress("TDparaA", &para[0]);
    myTree->SetBranchAddress("TDparaB", &para[1]);
    myTree->SetBranchAddress("TDparaC", &para[2]);
    myTree->SetBranchAddress("TDparaD", &para[3]);
    myTree->SetBranchAddress("TDparaG", &para[4]);
    myTree->SetBranchAddress("TDparaH", &para[5]);
    myTree->SetBranchAddress("TDisSmall", &isSmall);
    myTree->SetBranchAddress("TDisMedium", &isMedium);
    myTree->SetBranchAddress("TDisLarge", &isLarge);
    myTree->SetBranchAddress("TDmidpoint", &midpoint);

    TCanvas c1("c1","",800,600);
    c1.SetHighLightColor(10);
    TH1::AddDirectory(0);

    //Loop over entries
    while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
    {
       evtCtr++;
       
       if(!(int)isMedium) { continue; }

       cout <<"isSmall = " << isSmall 
	    <<"\nisMedium = " << isMedium
	    <<"\nisLarge = " << isLarge
	    << endl;

       if(isSmall) pulseSize = 0;
       if(isMedium) pulseSize = 1;
       if(isLarge) pulseSize = 2;

       //Setupt fit
       constructFit(pulseSize, para, func1, func2, midpoint);
       
       //Draw the pulses
       TH1F h_pulseBSN("PulseBSN", Form("event %d Zip# %d, Channel# %d, size %d", event, detectorNum, channelNum, pulseSize), nBins, 0.0, nBins);
       h_pulseBSN.SetStats(0);
       h_pulseBSN.SetMinimum(-5.0);
//             h_pulseBSN.SetMaximum(400.0);
       h_pulseBSN.Set(nBins, pulseBSN);
       c1.cd(1);
       h_pulseBSN.Draw();
       func1->SetLineColor(kBlue);
       func2->SetLineColor(kMagenta);
       if(isSmall || isMedium) func1->Draw("same");
       if(isLarge || isMedium) func2->Draw("same");

       c1.Update();

       //stupid! - to slow things down
       for(Int_t itr = 0; itr<2000; itr++)
       {
	  cout <<itr << endl;
       }

   }

   return;
}

//////////////////////////////////////////////////////////////////////////////////////////

void constructFit(int pulseSize, float* para, TF1* func1, TF1* func2, float midpoint)
{

   cout <<"Constructing Fit!" 
  	<<"\n fit para = " << para[0]
  	<<"\n fit para = " << para[1]
  	<<"\n fit para = " << para[2]
  	<<"\n fit para = " << para[3]
  	<<"\n fit para = " << para[4]
  	<< endl;

    //reconstructing fit for small pulses
    if(pulseSize == 0)
    {
       func1->SetParameter(0,para[0]);
       func1->SetParameter(1,para[1]);
       func1->SetParameter(2,para[2]); //start time
       func1->SetParameter(3,para[3]);
       func1->SetParameter(4,para[4]);

       double start = (para[2] < 0.0 ? 0.0 : para[2]);
       func1->SetRange(start, (double)nBins);
    }       

    //reconstructing fit for medium pulses
    if(pulseSize == 1)
    {
       func1->SetParameter(0,para[0]);
       func1->SetParameter(1,para[1]);
       func1->SetParameter(2,para[2]);
       func1->SetParameter(3,para[3]);
       func1->SetParameter(4,para[4]);

       double start = (para[2] < 0.0 ? 0.0 : para[2]);

       func1->SetParameter(0,para[0]);
       func1->SetParameter(1,para[1]);
       func1->SetParameter(2,para[2]);
       func1->SetParameter(3,para[3]);
       func1->SetParameter(4,para[4]);
       func1->SetRange(start, midpoint);
   
       func2->FixParameter(1,para[3]);
       func2->FixParameter(2,para[4]);
       func2->SetParameter(3,para[5]);
       func2->SetRange(midpoint,(double)nBins);
       
    }       

    //reconstructing fit for large pulses
    if(pulseSize == 2)
    {           
       cout <<"reconstructing large pulse!" << endl;
       
       //is this correct?
       func2->SetParameter(0,para[0]);
       func2->SetParameter(1,para[1]);
       func2->SetParameter(2,para[2]);
       func2->SetParameter(3,para[3]);
    }

    return;
}

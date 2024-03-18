static const Int_t nBins = 2048; 

void check_single()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;

   TFile f("testhisto.root");
   TTree* myTree = f.Get("pulseTree");

   //Variables
   Float_t pulse[nBins]; 
   Int_t series;
   Int_t event;
   Int_t detectorNum;
   Int_t channelNum;

   //Branch Addresses
   myTree->SetBranchAddress("event", &event);
   myTree->SetBranchAddress("detectorNum", &detectorNum);
   myTree->SetBranchAddress("channelNum", &channelNum);
   myTree->SetBranchAddress("pulse", pulse);

   TCanvas c1("c1","",800,600);
   c1.SetHighLightColor(10);
   TH1::AddDirectory(0);

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {

      //Draw the pulses
      TH1F h_pulse("Pulse", Form("Spectrum: event %d Det# %d, Channel# %d", event, detectorNum, channelNum), nBins, 0.0, nBins-1.0);
      h_pulse.SetStats(0);
//      h_pulse.SetMinimum(2000.0);
//      h_pulse.SetMaximum(2500.0);
      h_pulse.Set(nBins, pulse);
      c1.cd(1);
      h_pulse.Draw();

      c1.Update();

      //stupid! - to slow things down
      if(event > 280420)
      {
	 for(Int_t itr = 0; itr<50000; itr++)
	 {
	    cout <<itr << endl;
	 }
      }

      evtCtr++;
   }

   return;
}

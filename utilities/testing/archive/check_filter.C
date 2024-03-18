static const Int_t nBins = 2048; 

void check_filter()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;

   TFile f("testhisto.root");
   TTree* myTree = f.Get("pulseTree");

   //Variables
   Float_t pulse[nBins]; 
   Float_t pulseFlt[nBins]; 
   Int_t series;
   Int_t event;
   Int_t detectorNum;
   Int_t channelNum;

   //Branch Addresses
   myTree->SetBranchAddress("event", &event);
   myTree->SetBranchAddress("detectorNum", &detectorNum);
   myTree->SetBranchAddress("channelNum", &channelNum);
   myTree->SetBranchAddress("pulse", pulse);
   myTree->SetBranchAddress("pulseFlt", pulseFlt);

   TCanvas c1("c1","",1000,800);
   c1.SetHighLightColor(10);
   c1.Divide(1,2);
   TH1::AddDirectory(0);

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {
      evtCtr++;

      //      if(event != 280421) { continue; }

      //Draw the pulses
      TH1F h_pulse("Pulse", Form("Spectrum: event %d Det# %d, Channel# %d", event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulse.SetStats(0);
      h_pulse.SetMinimum(2000.0);
//      h_pulse.SetMaximum(2500.0);
      h_pulse.Set(nBins, pulse);
      c1.cd(1);
      h_pulse.Draw();

      TH1F h_pulseFlt("PulseFlt", Form("Spectrum: event %d Det# %d, Channel# %d", event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulseFlt.SetStats(0);
//      h_pulseFlt.SetMinimum(2000.0);
//      h_pulseFlt.SetMaximum(2500.0);
      h_pulseFlt.Set(nBins, pulseFlt);
      h_pulseFlt.SetLineColor(kBlue);
      h_pulseFlt.SetLineWidth(3);
      c1.cd(2);
      h_pulseFlt.Draw();

      c1.Update();

      //stupid! - to slow things down
      //      if(event > 280420)
      {
 	 for(Int_t itr = 0; itr<50000; itr++)
 	 {
	   cout <<itr << " last val = " << pulseFlt[nBins-1] << endl;
 	 }

	 //dump the filtered pulse
	 for(int pulseItr=0; pulseItr < nBins; pulseItr++)
	 {
	    cout <<"val = " << pulseFlt[pulseItr] << endl;
	 }
	 cout <<"Done!" << endl;
      }

   }

   return;
}

static const Int_t nBins = 2048; //Soudan, SUF

void check_basic()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;

   TFile f("testhisto.root");
   TTree* myTree = f.Get("pulseTree");

   //Variables
   Float_t pulseBSN[nBins]; 
   Float_t pulseBS[nBins]; 
   Float_t filteredPulsebw1[nBins];
   Float_t filteredPulsebw2[nBins];
   Int_t series;
   Int_t event;
   Int_t detectorNum;
   Int_t channelNum;
   float area;
   float rms;
   float max;
   float time1;
   float time2;
   float time3;

   //Branch Addresses
   myTree->SetBranchAddress("series", &series);
   myTree->SetBranchAddress("event", &event);
   myTree->SetBranchAddress("detectorNum", &detectorNum);
   myTree->SetBranchAddress("channelNum", &channelNum);
   myTree->SetBranchAddress("bsnPulse", pulseBSN);
   myTree->SetBranchAddress("bsPulse", pulseBS);
   myTree->SetBranchAddress("filtbw1Pulse", filteredPulsebw1);
   myTree->SetBranchAddress("filtbw2Pulse", filteredPulsebw2);

   TCanvas c1("c1","",800,800);
   c1.Divide(1,2);
   c1.SetHighLightColor(10);
   TH1::AddDirectory(0);

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {
      //Make the histograms
      TH1F h_pulseBS("PulseBS", Form("Spectrum: series %d, event %d Det# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulseBS.SetStats(0);
      h_pulseBS.Set(nBins, pulseBS);

      TH1F h_filtpulseBS("FilteredPulseBS", Form("Spectrum: series %d, event %d Det# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_filtpulseBS.SetStats(0);
      h_filtpulseBS.Set(nBins, filteredPulsebw1);
      h_filtpulseBS.SetLineColor(kMagenta);

      TH1F h_filtpulseBSNew("FilteredPulseBSNew", Form("Spectrum: series %d, event %d Det# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_filtpulseBSNew.SetStats(0);
      h_filtpulseBSNew.Set(nBins, filteredPulsebw2);
      h_filtpulseBSNew.SetLineColor(kBlue);

      //the x-axis
      TLine l;
      l.SetLineColor(kRed);
      

      //Draw the pulses
      c1.cd(1);
      h_pulseBS.SetMinimum(-10.0);
      h_pulseBS.SetMaximum(10.0);
      h_pulseBS.Draw();
      h_filtpulseBS.Draw("same");
      l.DrawLine(0,0,2048,0);

      c1.cd(2);
      h_pulseBS.Draw();
      h_filtpulseBSNew.Draw("same");
      l.DrawLine(0,0,2048,0);

      c1.Update();


      for(int ctr = 0; ctr < 2048; ctr++)
      {
 	 cout <<"ctr = " << ctr 
	      <<"\nfilt1 pulse = " << filteredPulsebw1[ctr] 
	      <<"\nfilt2 pulse = " << filteredPulsebw2[ctr] 
	      << endl;
      }

      //stupid! - to slow things down
      for(Int_t itr = 0; itr<10000; itr++)
      {
	 cout <<itr << endl;
      }

      evtCtr++;
   }

   return;
}

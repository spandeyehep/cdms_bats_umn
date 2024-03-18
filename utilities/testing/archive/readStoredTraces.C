void readStoredTraces()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;

   //   TFile f("pulseDump_170319_1616_F0002.root");
   TFile f("testhisto.root");
   TTree* myTree = f.Get("pulseTree");

   //Variables
   //static const Int_t nBins = 1024; //Soudan Veto
   static const Int_t nBins = 2048; //Soudan, SUF
   //static const Int_t nBins = 4096; //UCB
   Float_t pulse[nBins]; 
   Float_t pulseBSN[nBins]; 
   Float_t pulseFilt[nBins]; 
   Float_t pulseFFT[nBins]; 
   Float_t pulseFiltFFT[nBins]; 
   Int_t series;
   Int_t event;
   Int_t detectorNum;
   Int_t channelNum;

   //Histograms
   TH1F* h_detNum = new TH1F("DetectorIDNum","DetectorIDNum", 50, 0, 50);
   TH1F* h_chanNum = new TH1F("ChannelIDNum","ChannelIDNum", 10, 0, 10);

   //Branch Addresses
   myTree->SetBranchAddress("series", &series);
   myTree->SetBranchAddress("event", &event);
   myTree->SetBranchAddress("detectorNum", &detectorNum);
   myTree->SetBranchAddress("channelNum", &channelNum);
   myTree->SetBranchAddress("rawPulse", pulse);
   myTree->SetBranchAddress("bsnPulse", pulseBSN);
   myTree->SetBranchAddress("filtPulse", pulseFilt);
   myTree->SetBranchAddress("fftPulse", pulseFFT);
   myTree->SetBranchAddress("filtfftPulse", pulseFiltFFT);

   TCanvas c1("c1","",800,1000);
   //   c1.Divide(2,1);
   c1.Divide(1,2);
   c1.SetHighLightColor(10);
   TH1::AddDirectory(0);

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {
      //numbers 1-30 for the 30 zips
      h_detNum->Fill(detectorNum);

      //0=Qi, 1=Qo, 2=phononA, 3=phononB, 4=phononC, 5=phononD
      h_chanNum->Fill(channelNum);

      //Draw the pulses
      TH1F h_pulse("Pulse", Form("Raw: Zip# %d, Channel# %d", detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulse.SetStats(0);
      h_pulse.SetMinimum(-5.0);
      h_pulse.SetMaximum(4000.0);
      h_pulse.Set(nBins, pulse);
      //      c1.cd(1);
      //h_pulse.Draw();

      TH1F h_pulseBSN("PulseBSN", Form("Spectrum: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulseBSN.SetStats(0);
      h_pulseBSN.SetMinimum(-5.0);
      h_pulseBSN.SetMaximum(95.0);
      h_pulseBSN.Set(nBins, pulseBSN);
      c1.cd(1);
      h_pulseBSN.Draw();

      TH1F h_pulseFilt("PulseFilt", Form("Spectrum: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulseFilt.SetStats(0);
      h_pulseFilt.SetMinimum(-5.0);
      h_pulseFilt.SetMaximum(95.0);
      h_pulseFilt.Set(nBins, pulseFilt);
      h_pulseFilt.SetLineColor(kBlue);
      c1.cd(1);
      h_pulseFilt.Draw("same");

      TLegend legend(0.62, 0.62, 0.92, 0.92);
      legend.AddEntry(&h_pulseBSN, "Unfiltered", "p");
      legend.AddEntry(&h_pulseFilt, "Filtered", "p");

      TH1F h_pulseFFT("PulseFFT", Form("PowerSpec: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, 625.0e3); 
      h_pulseFFT.SetStats(0);
      h_pulseFFT.SetMinimum(1.0e-2);
      h_pulseFFT.SetMaximum(1.0e8);
      h_pulseFFT.Set(nBins, pulseFFT);
      //      h_pulseFFT.GetXaxis()->SetLimits(1.0, 4*nBins);

      TH1F h_pulseFiltFFT("PulseFiltFFT", Form("PowerSpec: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, 625.0e3);
      h_pulseFiltFFT.SetStats(0);
      h_pulseFiltFFT.SetMinimum(1.0e-2);
      h_pulseFiltFFT.SetMaximum(1.0e8);
      //h_pulseFiltFFT.GetXaxis().SetLimits(1.0, 4*nBins);
      h_pulseFiltFFT.Set(nBins, pulseFiltFFT);
      c1.cd(2);
      gPad->SetLogy();
      //      gPad->SetLogx();
      h_pulseFFT.Draw();
      h_pulseFiltFFT.SetLineColor(kBlue);
      h_pulseFiltFFT.Draw("same");

      c1.Update();

      //stupid! - to slow things down
      for(Int_t itr = 0; itr<20000; itr++)
	{
	  cout <<itr << endl;
	}

      evtCtr++;
   }

   //More Histograms
   TCanvas* mycan = new TCanvas("mycan", "mycan");
   mycan->Divide(2,2);
   mycan->cd(1);
   h_detNum->DrawCopy();
   mycan->cd(2);
   h_chanNum->DrawCopy();

   return;
}

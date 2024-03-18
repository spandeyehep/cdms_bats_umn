void getPulseTemplate(TH1F& h_pulsetemp, int detectorNum);
void shiftHisto(TH1F& h, int idelay);

static const int nBins = 2048; //for Soudan only

void checkOF()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;
   float dt = 1.0/1.25e6;
   float trigTime = 500; //FIXME, only approx!

   TFile f("testhisto.root");
   TTree* myTree = f.Get("pulseTree");

   //Variables
   Float_t pulseBSN[nBins]; 
   Int_t series;
   Int_t event;
   Int_t detectorNum;
   Int_t channelNum;
   Float_t OFAmp;
   Float_t OFDelay;

   //Branch Addresses
   myTree->SetBranchAddress("series", &series);
   myTree->SetBranchAddress("event", &event);
   myTree->SetBranchAddress("detectorNum", &detectorNum);
   myTree->SetBranchAddress("channelNum", &channelNum);
   myTree->SetBranchAddress("bsnPulse", pulseBSN);
   myTree->SetBranchAddress("OFAmp", &OFAmp);
   myTree->SetBranchAddress("OFDelay", &OFDelay);

   TCanvas c1("c1","",800,600);
   c1.SetHighLightColor(10);
   TH1::AddDirectory(0);

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {
      //Draw the pulses
      TH1F h_pulseBSN("PulseBSN", Form("Spectrum: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, channelNum), nBins, 0.0, nBins);
      h_pulseBSN.SetStats(0);
      h_pulseBSN.SetMinimum(-5.0);
      //      h_pulseBSN.SetMaximum(400.0);
      h_pulseBSN.Set(nBins, pulseBSN);
      c1.cd(1);
      h_pulseBSN.Draw();

      //the fit
      TH1F h_fit("Fit", Form("OF Fit", series, event, detectorNum, channelNum), nBins, 0.0, nBins); 
      getPulseTemplate(h_fit, detectorNum);
      shiftHisto(h_fit, floor(OFDelay/dt)+trigTime);
      h_fit.Scale(OFAmp);
      h_fit.SetLineColor(kBlue);
      h_fit.Draw("same");

      c1.Update();

      //stupid! - to slow things down
      for(Int_t itr = 0; itr<20000; itr++)
	{
	  cout <<itr << endl;
	}

      evtCtr++;

   }

   return;
}

//////////////////////////////////////////////////////////////////////////////////////////

void getPulseTemplate(TH1F& h_pulsetemp, int detectorNum)
{
   //Temp! using the template pulsetemp as the data
   TFile f("templates/NoiseAndTemplates_1703191616.root");
   TTree* pulsetempTree = (TTree*)f.Get(Form("zip%d",detectorNum));

   //Set Branch Addresses
   double pulsetemp[nBins];
   pulsetempTree->SetBranchAddress("PA_template_time", pulsetemp);
   pulsetempTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
     h_pulsetemp.SetBinContent(binItr+1, (float)pulsetemp[binItr]);
   }
   
   return;
}

void shiftHisto(TH1F& h, int idelay)
{
  TH1F* aTempHisto = (TH1F*)h.Clone("aCopy");

  //make shifted histo, aTempHisto 
  for(int binItr = 0; binItr < h.GetNbinsX(); binItr++)
    {
      if(binItr<idelay) 
	aTempHisto->SetBinContent(binItr+1, 0.0); 
      else
	aTempHisto->SetBinContent(binItr+1, h.GetBinContent(binItr-idelay+1));
    }

  //copy contents of aTempHist into h
  for(int binItr = 0; binItr < h.GetNbinsX(); binItr++)
    {
      h.SetBinContent(binItr+1, aTempHisto->GetBinContent(binItr+1));
    }

  return;
}

void constructFit(TH1F& h_pulsetemp, int detectorNum, float OFAmpQI, float OFAmpQO, const string& chanName);
void shiftHisto(TH1F& h, int idelay);

static const int nBins = 2048; //for Soudan only

void checkOFX()
{
   Int_t maxEvents = 1e9;
   Int_t evtCtr = 0;
   float dt = 1.0/1.25e6;
   float trigTime = 512; //FIXME, only approx!

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

   TCanvas c1("c1","",800,1000);
   c1.Divide(1,2);
   c1.SetHighLightColor(10);
   TH1::AddDirectory(0);

   Float_t OFAmpQI;
   Float_t OFAmpQO;
   Int_t   chanQI;
   Int_t   chanQO;
   Float_t pulseBSNQI[nBins]; 
   Float_t pulseBSNQO[nBins]; 

   //Loop over entries
   while(myTree->GetEntry(evtCtr) && evtCtr < maxEvents)
   {
      TH1F h_pulseBSNQI("PulseBSNQI", Form("Spectrum: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, chanQI), nBins, 0.0, nBins);

      TH1F h_pulseBSNQO("PulseBSNQO", Form("Spectrum: series %d, event %d Zip# %d, Channel# %d", series, event, detectorNum, chanQO), nBins, 0.0, nBins);


      //check if evtCtr is odd - of so assume we have a pair
      if(evtCtr % 2 != 0)
      {	 
	 //store QO values
	 OFAmpQO = OFAmp;
	 chanQO = channelNum;
	 for(int binItr = 0; binItr < nBins; binItr++)
	    pulseBSNQO[binItr] = pulseBSN[binItr];

	 //Draw the pulses if a pair is found

	 //draw QI
	 h_pulseBSNQI.SetStats(0);
	 h_pulseBSNQI.SetMinimum(-5.0);
	 h_pulseBSNQI.SetMaximum(800.0);
	 h_pulseBSNQI.Set(nBins, pulseBSNQI);
	 c1.cd(1);
	 h_pulseBSNQI.Draw();
	 
	 //the QI fit
	 TH1F h_fitQI("Fit", Form("OF Fit", series, event, detectorNum, channelNum), nBins, 0.0, nBins); 
	 constructFit(h_fitQI, detectorNum, OFAmpQI, OFAmpQO, "QI");
	 shiftHisto(h_fitQI, floor(OFDelay/dt)+trigTime);
//	 h_fitQI.Scale(OFAmpQI);
	 h_fitQI.SetLineColor(kBlue);
	 h_fitQI.Draw("same");

	 //draw QO
	 h_pulseBSNQO.SetStats(0);
	 h_pulseBSNQO.SetMinimum(-5.0);
	 h_pulseBSNQO.SetMaximum(400.0);
	 h_pulseBSNQO.Set(nBins, pulseBSNQO);
	 c1.cd(2);
	 h_pulseBSNQO.Draw();
	 
	 //the QO fit
	 TH1F h_fitQO("Fit", Form("OF Fit", series, event, detectorNum, channelNum), nBins, 0.0, nBins); 
	 constructFit(h_fitQO, detectorNum, OFAmpQI, OFAmpQO, "QO");
	 shiftHisto(h_fitQO, floor(OFDelay/dt)+trigTime);
//	 h_fitQO.Scale(OFAmpQO);
	 h_fitQO.SetLineColor(kBlue);
	 h_fitQO.Draw("same");
	 
	 c1.Update();
	 
	 //stupid! - to slow things down
	 for(Int_t itr = 0; itr<20000; itr++)
	 {
	    cout <<itr << endl;
	 }
      }
      else
      {
	 OFAmpQI = OFAmp;
	 chanQI = channelNum;
	 for(int binItr = 0; binItr < nBins; binItr++)
	    pulseBSNQI[binItr] = pulseBSN[binItr];
      }

      evtCtr++;

   }

   return;
}

//////////////////////////////////////////////////////////////////////////////////////////

void constructFit(TH1F& h_pulsetemp, int detectorNum, float AmpQI, float AmpQO, const string& chanName)
{
   //Temp! using the template pulsetemp as the data
   TFile f("templates/NoiseAndTemplates_1703191616.root");
   TTree* pulsetempTree = (TTree*)f.Get(Form("zip%d", detectorNum));

   //Set Branch Addresses
   double pulsetempQI[nBins];
   double pulsetempQO[nBins];
   double pulsetempQIX[nBins];
   double pulsetempQOX[nBins];
   pulsetempTree->SetBranchAddress("QI_template_time", pulsetempQI);
   pulsetempTree->SetBranchAddress("QIX_template_time", pulsetempQIX);
   pulsetempTree->SetBranchAddress("QO_template_time", pulsetempQO);
   pulsetempTree->SetBranchAddress("QOX_template_time", pulsetempQOX);
   pulsetempTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      if(chanName == "QI")
	 h_pulsetemp.SetBinContent(binItr+1, (float)pulsetempQI[binItr]*AmpQI + (float)pulsetempQIX[binItr]*AmpQO);

      if(chanName == "QO")
	 h_pulsetemp.SetBinContent(binItr+1, (float)pulsetempQO[binItr]*AmpQO + (float)pulsetempQOX[binItr]*AmpQI);     
   }
   
   return;
}

void shiftHisto(TH1F& h, int idelay)
{
  TH1F* aTempHisto = (TH1F*)h.Clone("aCopy");
 
  //make shifted histo, aTempHisto 
  for(int binItr = 0; binItr < h.GetNbinsX(); binItr++)
    {
      //positive shift 
      if(idelay >= 0)
      {
	 if(binItr<idelay) 
	    aTempHisto->SetBinContent(binItr+1, 0.0); 
	 else
	    aTempHisto->SetBinContent(binItr+1, h.GetBinContent(binItr-idelay+1));
      }
      //negative shift
      else
      {
	 if( (binItr-idelay) < h.GetNbinsX() )
	    aTempHisto->SetBinContent(binItr+1, h.GetBinContent(binItr-idelay+1));
	 else
	    aTempHisto->SetBinContent(binItr+1, 0.0);
      }

    }

  //copy contents of aTempHist into h
  for(int binItr = 0; binItr < h.GetNbinsX(); binItr++)
    {
      h.SetBinContent(binItr+1, aTempHisto->GetBinContent(binItr+1));
    }

  return;
}

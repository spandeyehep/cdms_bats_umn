//Standard Libaries
#include <iostream>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries
#include "RawDataReader.h"
#include "ExtFilesManager.h"

#include "AdminData.h"
#include "PulseData.h"
#include "EventBuilder.h"

//Temp
#include "PulseTools.h"

using namespace std;

/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char* argv[]){


  // ================================================
  // Configuration from Config/Info file and/or 
  // input to main
  // ===============================================
  
   ExtFilesManager myExtData;

  // temporary assignements....   

  // raw data fine name 
  string rawDataFilename = argv[1]; //filename


  // configuration file (temporary on argc[4])
  string configFile = (argc > 4 ? argv[4] : "configSoudanData.Default");
  myExtData.ReadConfigFile("configuration/" + configFile);
 
  // should this go here or in EB? TEMP hardcoding, should come from config
  myExtData.ReadFilterFile("templates/NoiseandTemplates_1703191616.root");

  // info file
  // TEMP, use 180326_1455.info
  
  if (myExtData.GetProcessingRecord()->DoReadINFO())
                 myExtData.ReadInfoFile("180326_1455.info");


  int maxEventsDefault = myExtData.GetProcessingRecord()->GetMaxEvents();
  int maxEvents = (argc > 2 ? atoi(argv[2]) : maxEventsDefault);
  Int_t nBins = (argc > 3 ? atoi(argv[3]) : 2048);  //Temp

  //=================================================
  // Initialize Event Builder
  //=================================================
  
  //Input setup
  EventBuilder eventBuilder(myExtData, rawDataFilename); 

  //=========================================
  // Initialize Output Variables Lists/Trees
  //============================================
    
  //Tree to store traces - Temp
  //  Int_t nBins = 2048;
  TFile f_out("testhisto.root", "recreate");

  Float_t pulseBSN[nBins];
  Int_t detectorNum = 999;
  Int_t channelNum = 999;
  Int_t series = 999;
  Int_t event = 999;
  Float_t TDparaA, TDparaB, TDparaC, TDparaD, TDparaG, TDparaH;
  Float_t TDchisq, TDdof,TDisSmall, TDisMedium, TDisLarge;
  Float_t TDmidpoint, TDisFit;
  TTree pulseTree("pulseTree","a test tree");
  pulseTree.Branch("series", &series, "series/I");
  pulseTree.Branch("event", &event, "event/I");
  pulseTree.Branch("detectorNum", &detectorNum, "detectorNum/I");
  pulseTree.Branch("channelNum", &channelNum, "channelNum/I");
  pulseTree.Branch("bsnPulse", pulseBSN, Form("bsnPulse[%d]/F",nBins));
  pulseTree.Branch("TDparaA", &TDparaA, "TDparaA/F");
  pulseTree.Branch("TDparaB", &TDparaB, "TDparaB/F");
  pulseTree.Branch("TDparaC", &TDparaC, "TDparaC/F");
  pulseTree.Branch("TDparaD", &TDparaD, "TDparaD/F");
  pulseTree.Branch("TDparaG", &TDparaG, "TDparaG/F");
  pulseTree.Branch("TDparaH", &TDparaH, "TDparaH/F");
  pulseTree.Branch("TDchisq", &TDchisq, "TDchisq/F");
  pulseTree.Branch("TDdof", &TDdof, "TDdof/F");
  pulseTree.Branch("TDisSmall", &TDisSmall, "TDisSmall/I");
  pulseTree.Branch("TDisMedium", &TDisMedium, "TDisMedium/I");
  pulseTree.Branch("TDisLarge", &TDisLarge, "TDisLarge/I");
  pulseTree.Branch("TDisFit", &TDisFit, "TDisFit/I");
  pulseTree.Branch("TDmidpoint", &TDmidpoint, "TDmidpoint/F");

  //=======================================================
  
  //Loop over events
  int evtCtr = 0;
  while(evtCtr < maxEvents && eventBuilder.ReadNextEvent() != 0)
  {
     // ======= Admin, Veto, Trigger, etc.  ========

    series = eventBuilder.GetAdmin().GetSeries();
    event = eventBuilder.GetAdmin().GetEvent(); 

     cout <<"Series Number = " << series
	  <<"\nEvent Number = " << event
	  << endl;

     // ==============================
     // Start ZIP Analysis
     // ==============================

     // ============== Pulse Analysis Starts Here ==============	 

      eventBuilder.SetPulseSaturationBools("allzip");  //pulse tools	   
      eventBuilder.SetPulseMeanBaselines("allzip");
      eventBuilder.CreatePulseBaselineSubNorm("allzip"); //pulse tools

      eventBuilder.CalcPulseMaximum("allzip");
      eventBuilder.CalcPulseRMS("allzip");

     //  =========  filtering ==========
      eventBuilder.CreateFilteredPulses("butterworth", "allzip"); //what parameter set here? versus config
      eventBuilder.CalcWalkTimes("allzip", "filtered"); //go into pulse tools, percentage read from configuration 
      eventBuilder.CalcPulseArea("allzip", "filtered"); //option to do on filtered pulses?

     //  ===== optimal filtering  ======
//     eventBuilder.DoOptimalFilterPhonon();
     
     //  ===== time domain =======
     eventBuilder.DoTimeDomainFit("allphonon");

     //  ===== zip timing (w/ dmm) =======


     // ============== Data Writeout  ==============	 

      std::vector<PulseData> zipCollection;
      eventBuilder.FillSubPulseCollection(zipCollection, "allphonon");

      cout <<"the size of the zip collection = " << zipCollection.size() << endl;

      //looping over pulses
      for(uint pulseItr = 0; pulseItr < zipCollection.size(); pulseItr++)
      {
	 PulseData aPulseData = zipCollection[pulseItr];

	 detectorNum = aPulseData.GetDetectorNum();
	 channelNum = aPulseData.GetDetectorChannel();

// 	 cout <<"MeanBaseline = " << aPulseData.GetMeanBaseline() 
// 	      <<"\nChannel = " << aPulseData.GetDetectorChannel() 
// 	      <<"\nisSaturated = " << aPulseData.IsSaturated()
// 	      << endl;

	 if(aPulseData.GetTimeDomain().IsFit())
	 {
	    std::vector<double> fitResults = (aPulseData.GetTimeDomain()).GetParameters();

	    TDparaA = fitResults[0];
	    TDparaB = fitResults[1];
	    TDparaC = fitResults[2];
	    TDparaD = fitResults[3];
	    TDparaG = fitResults[4];
	    TDparaH = fitResults[5]; //for medium pulses only
	    
	    TDchisq = aPulseData.GetTimeDomain().GetChi2();
	    TDdof = aPulseData.GetTimeDomain().GetDOF();
	    TDisSmall = aPulseData.GetTimeDomain().IsSmall();
	    TDisMedium = aPulseData.GetTimeDomain().IsMedium();
	    TDisLarge = aPulseData.GetTimeDomain().IsLarge();

	    TDmidpoint = aPulseData.GetTimeDomain().GetMidpoint(); 
	    TDisFit = aPulseData.GetTimeDomain().IsFit(); 

	    for(int itr=0; itr<nBins; itr++)
	    {
	       pulseBSN[itr] = (aPulseData.GetBaselineSubNormPulse())[itr];
	    }

	    cout <<"\nchisq/dof = " << TDchisq/TDdof
		 <<"\npulsesize = " << TDisSmall <<TDisMedium << TDisLarge
		 <<"\nparaA = " << TDparaA
		 <<"\nparaB = " << TDparaB
		 <<"\nparaC = " << TDparaC
		 <<"\nparaD = " << TDparaD
		 <<"\nparaG = " << TDparaG
		 <<"\nparaH = " << TDparaH
		 << endl;

	    
	    pulseTree.Fill();

	 }


      }

     // =============End of ZIP analysis  ============

      cout <<"eventCtr = " << evtCtr << endl;
      evtCtr++;

     }

  //==========Done looping over events!=================

  f_out.cd();
  pulseTree.Write(); //Temp
  f_out.Close(); //Temp

  return 0;

} //end main()   DONE!!!



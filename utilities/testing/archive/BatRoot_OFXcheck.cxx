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
  
  if (myExtData.GetProcessingRecord()->DoRead("INFO_FILE"))
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
  Float_t OFAmp;
  Float_t OFDelay;
  TTree pulseTree("pulseTree","a test tree");
  pulseTree.Branch("series", &series, "series/I");
  pulseTree.Branch("event", &event, "event/I");
  pulseTree.Branch("detectorNum", &detectorNum, "detectorNum/I");
  pulseTree.Branch("channelNum", &channelNum, "channelNum/I");
  pulseTree.Branch("bsnPulse", pulseBSN, Form("bsnPulse[%d]/F",nBins));
  pulseTree.Branch("OFAmp", &OFAmp, "OFAmp/F");
  pulseTree.Branch("OFDelay", &OFDelay, "OFDelay/F");
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

     //  =========  filtering ==========
      //   eventBuilder.CreateFilteredPulses("butterworth", "allzip"); //what parameter set here? versus config
//    eventBuilder.CalcWalkTimes("allzip", "filtered"); //go into pulse tools, percentage read from configuration 
     
     //  ===== optimal filtering  ======
     eventBuilder.DoOptimalFilterChargeX("allcharge");
     
     //  ===== time domain =======
//     eventBuilder.DoTimDomainFit("allzip");

     //  ===== zip timing (w/ dmm) =======


     // ============== Data Writeout  ==============	 

      std::vector<PulseData> zipCollection;
      eventBuilder.FillSubPulseCollection(zipCollection, "allcharge");

      cout <<"the size of the zip collection = " << zipCollection.size() << endl;

      //looping over pulses
      for(uint pulseItr = 0; pulseItr < zipCollection.size(); pulseItr++)
      {
	 PulseData aPulseData = zipCollection[pulseItr];

	 detectorNum = aPulseData.GetDetectorNum();
	 channelNum = aPulseData.GetDetectorChannel();

	 cout <<"\nMeanBaseline = " << aPulseData.GetMeanBaseline() 
	      <<"\nChannel = " << aPulseData.GetDetectorChannel() 
	      <<"\nisSaturated = " << aPulseData.IsSaturated()
	      << endl;

	 //Getting OF values through rq map - read only ability here!
	 std::map<string,double> ofRQList = (aPulseData.GetPulseAnalysis("OptimalFilterChargeX")).GetRQList();

	 cout <<"Channel = " << aPulseData.GetChannelName()
	      <<"\nAmp = " << ofRQList["OFvolts"]
	      <<"\nDelay = " << ofRQList["OFdelay"]
	      << endl;

	 OFAmp = ofRQList["OFvolts"];
	 OFDelay = ofRQList["OFdelay"];

	 for(int itr=0; itr<nBins; itr++)
	   {
	     pulseBSN[itr] = (aPulseData.GetBaselineSubNormPulse())[itr];
	   }

	 pulseTree.Fill();

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



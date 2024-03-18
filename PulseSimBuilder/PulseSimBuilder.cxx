//////////////////////////////////////////////////////////////////////////////// 
// main()
// Author: A. Anderson
//
// Description: Tool for creating pulse template library for the cdmsbats pulse
// scaling and noise injection routine.
//
// File Import By: A. Anderson
// Creation Date: Nov. 17, 2008
//
// Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

//Standard Libaries
#include <iostream>
#include <list>
#include "time.h"
#include "string.h"
#include <sstream>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"

//CDMS Libraries
#include "AdminData.h"
#include "RawDataReader.h"
#include "PulseData.h"
#include "PulseEvtBuilder.h"
#include "UserDataManager.h"

#include "DetectorConfigManager.h"
#include "PulseTools.h"

using namespace std;

/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char* argv[]){
  // check arguments
  if(argc < 4)
    cout << "Usage: PulseSimBuilder [input file] [dir. containing filter files] [output file (must have .root extension!)]" << endl;

  // create the output file and tree
  TFile templateFile(argv[3], "RECREATE");
  map<int, TTree*> treeMap;

  // variables to write to tree
  //vector<double> h_PAS1, h_PBS1, h_PCS1, h_PDS1, h_PAS2, h_PBS2, h_PCS2, h_PDS2, 
  //               h_PS1, h_PS2, h_PT, h_QIS1, h_QOS1, h_QIS2, h_QOS2;
  std::map<int, std::map<string, vector<double> > > traceMap;
  vector<double> chanAmp;
  double SIMEventNumber, SIMSeriesNumber, ptnorm;
  vector<string> chanStr;

  //cdmsbats directory
   string cdmsbatsdir =
       getenv("CDMSBATSDIR") ? getenv("CDMSBATSDIR") : "./";
 
  
   //batroot directory
   string batrootdir = cdmsbatsdir + "/BatRoot";
    
   // processing/analysis root directory
   string batroot_settings = cdmsbatsdir + "/UserSettings/BatRootSettings";
  

   //processing files directory
   string batroot_proc_default = batroot_settings + "/processing";
   string batroot_proc =
       getenv("BATROOT_PROC") ? getenv("BATROOT_PROC")
       : batroot_proc_default;


   //analysis files directory
   string batroot_const_default = batroot_settings + "/analysis";
   string batroot_const =
       getenv("BATROOT_CONST") ? getenv("BATROOT_CONST")
       : batroot_const_default;


   //detector status files directory
   string batroot_detstatus_default = batroot_settings + "/detector_status";
   string batroot_detstatus =
       getenv("BATROOT_DETSTATUS") ? getenv("BATROOT_DETSTATUS")
       : batroot_detstatus_default;

  
  // -- analysis and processing configurations --
   
  // autodetect the location based on series number and then set the default configuration file
  string defaultUserOptionsFile = "processingSoudanData.SuperCDMS.Default";
  string defaultAnalysisConfigFile = "configSoudanData.SuperCDMS.Default";
  string defaultDetectorStatusFile = "detectorStatus.SuperCDMS";
  //string inputSeries = "01130308_2145";


  /*TFile templateFile(argv[3], "RECREATE");
  vector<TTree*> treeVector;
  for(int jzip = 1; jzip <= 15; jzip++)
  {
      treeVector.push_back(new TTree(Form("zip%d", jzip), Form("%d", jzip)));

      // create branches
      treeVector[jzip-1]->Branch("PAS1", &h_PAS1);
      treeVector[jzip-1]->Branch("PBS1", &h_PBS1);
      treeVector[jzip-1]->Branch("PCS1", &h_PCS1);
      treeVector[jzip-1]->Branch("PDS1", &h_PDS1);
      treeVector[jzip-1]->Branch("PAS2", &h_PAS2);
      treeVector[jzip-1]->Branch("PBS2", &h_PBS2);
      treeVector[jzip-1]->Branch("PCS2", &h_PCS2);
      treeVector[jzip-1]->Branch("PDS2", &h_PDS2);
      treeVector[jzip-1]->Branch("PT", &h_PT);
      treeVector[jzip-1]->Branch("PS1", &h_PS1);
      treeVector[jzip-1]->Branch("PS2", &h_PS2);
      treeVector[jzip-1]->Branch("QIS1", &h_QIS1);
      treeVector[jzip-1]->Branch("QOS1", &h_QOS1);
      treeVector[jzip-1]->Branch("QIS2", &h_QIS2);
      treeVector[jzip-1]->Branch("QOS2", &h_QOS2);
      treeVector[jzip-1]->Branch("chanStr", &chanStr);
      treeVector[jzip-1]->Branch("chanAmps", &chanAmp);
      treeVector[jzip-1]->Branch("PTnorm", &ptnorm);
      treeVector[jzip-1]->Branch("SIMEventNumber", &SIMEventNumber);
      treeVector[jzip-1]->Branch("SIMSeriesNumber", &SIMSeriesNumber);
      }*/

  // open the input file
  std::ifstream inputFile;
  inputFile.open(argv[1], std::ifstream::in);

  int jevt = 0;

  // loop over events
  while(inputFile.peek() != EOF)
  {
    // loop over detectors in this event
    while(inputFile.peek() != '*')
    {
      // allocate variables
      int detnum, evtnum;
      string seriesnum;

      // get line with file info
      inputFile >> detnum >> seriesnum >> evtnum;
      inputFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
      cout << "Event #" << jevt << ": " << detnum << ", " << seriesnum << ", " << evtnum << endl;

      // parse event number into dump number and event number in dump
      int dumpnum = evtnum / 10000;
      int jevt = evtnum % 10000;

      // put the event and series numbers into the variables to write to ROOT file
      SIMEventNumber = double(evtnum);
      string seriesstr = string(seriesnum);
      seriesstr.erase(remove(seriesstr.begin(), seriesstr.end(),'_'), seriesstr.end());
      istringstream convert(seriesstr);
      convert >> SIMSeriesNumber;

      // construct raw filename
      stringstream ss;
      int nZeros = 3 - (int) log10((double) dumpnum);
      string dumpName = "F";
      for(int zCtr=0; zCtr < nZeros; zCtr++)
        dumpName += "0";
      ss << dumpnum;
      dumpName += ss.str();
      string rawDataFilename = Form("atto/data1/%s/%s_%s", seriesnum.c_str(), seriesnum.c_str(), dumpName.c_str());

      UserDataManager myUserData;  

      // if desired, overwrite default processing options with fourth argument
      string userOptionsFile = defaultUserOptionsFile;
      // now read the processing file
      myUserData.ReadFile(batroot_proc + "/" + userOptionsFile); 
      
      // if desired, overwrite default analysis config with fifth argument
      string configFile = defaultAnalysisConfigFile;
      // now read the analysis configuration file
      myUserData.ReadFile(batroot_const + "/" + configFile);

      
      //
      // ===== GPIB change log file ===== 
      //   
      GpibDataManager myGpibData;
      string gpibFile = myUserData.GetPath("GPIB_FILE") + "gpib_states_changed.log"; 
      if (myUserData.DoRead("GPIB_FILE"))
	  myGpibData.ReadFile(gpibFile);

      // declare some random manager stuff
      DetectorConfigManager detConfigManager(myUserData, rawDataFilename);
      map<int, int> detectorMap = detConfigManager.GetDetectorMap();


      // get the detector status information 
      if (myUserData.DoRead("DET_STATUS_FILE"))
      {
	  myUserData.ReadFile(batroot_detstatus + "/" + defaultDetectorStatusFile);
	  myUserData.ConstructZipRQList(detectorMap);
	  myUserData.DoCalcDetectorStatus(seriesnum);
	  myUserData.FillBrokenChannelLists();
      }
      PulseEvtBuilder eventBuilder(myUserData, detConfigManager, rawDataFilename); 

      FilterDataManager myFilterData(detectorMap);
      string noisePrefix = myUserData.GetPrefix("NOISE_PREFIX");
      string noisePath = argv[2];
      string noiseFile = noisePath + "/Prodv5-3_Filter_" + seriesnum  + ".root";
      if(myUserData.DoRead("FILTER_FILE"))
	  myFilterData.ReadFile(noiseFile);
      if( myUserData.DoRead("FILTER_FILE") ) 
	  eventBuilder.RegisterFilter(myFilterData);
      // store parameters from filter file
      if(myUserData.GetIntParameter("WRITE_FILTER_INFO") && myUserData.DoRead("FILTER_FILE")) 
	  myFilterData.DoCalc(detConfigManager);

      eventBuilder.ReadEventN(jevt-1);
      
      // calculate the basic pulse quantities and get the results
      eventBuilder.DoBasicPulseCalc(detnum);      
      if( myUserData.DoAlgorithm(detnum, "phonon", "OptimalFilterPhonon")) 
	  eventBuilder.DoOptimalFilterPhonon(detnum, "phonon");
      if( myUserData.DoAlgorithm(detnum, "PT", "OptimalFilterPhononNS") ) 
	  eventBuilder.DoOptimalFilterPhononNS(detnum, "phonon");
      if( myUserData.DoAlgorithm(detnum, "charge", "OptimalFilterCharge2X2") )
	  eventBuilder.DoOptimalFilterCharge2X2(detnum);


      vector<PulseData> pulseCollection;
      eventBuilder.FillSingleZipPulseCollection(pulseCollection, detnum);

      // get the PT normalization by looping over the
      // pulses until we find the PT channel
      const int nChan = pulseCollection.size();
      int jChan = 0;
      ptnorm = 0;
      while(jChan < nChan && pulseCollection[jChan].GetChannelName() != "PT")
	  jChan++;
	  
      if(jChan < nChan && pulseCollection[jChan].GetChannelName() == "PT")
      {
   	  ptnorm = pulseCollection[jChan].GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFamps");
      }
      else
      {
	  cout << "PulseSimBuilder: ERROR! Could not find PT pulse in pulse collection for zip " << detnum << endl;
          exit(EXIT_FAILURE);
      }

      // check whether output tree has been created and create if needed
      if(treeMap.find(detnum) == treeMap.end())
      {
	  std::cout << "CREATING NEW TREE!!" << std::endl;
	  if(traceMap.find(detnum) != traceMap.end())
	      traceMap[detnum].clear();

	  treeMap.insert(std::pair<int, TTree*>(detnum, new TTree(Form("zip%d", detnum), Form("%d", detnum))));

	  // loop over channels of detector
	  traceMap.insert(std::pair<int, std::map<string, vector<double> > >(detnum, std::map<string, vector<double> >()));
	  for(int jChan = 0; jChan < nChan; jChan++)
	  {
	      string chanName = pulseCollection[jChan].GetChannelName();
	      traceMap[detnum].insert(std::pair<string, vector<double> >(chanName, vector<double>()));
	      treeMap[detnum]->Branch(chanName.c_str(), &traceMap[detnum][chanName]);
	      treeMap[detnum]->SetDirectory(&templateFile);
	  }
	  
	  treeMap[detnum]->Branch("chanStr", &chanStr);
	  treeMap[detnum]->Branch("chanAmps", &chanAmp);
	  treeMap[detnum]->Branch("PTnorm", &ptnorm);
	  treeMap[detnum]->Branch("SIMEventNumber", &SIMEventNumber);
	  treeMap[detnum]->Branch("SIMSeriesNumber", &SIMSeriesNumber);
      }

      // loop over the pulse collection to do the calculations
      for(int jChan = 0; jChan < nChan; jChan++)
      {
	  // check whether branch has been created and create if needed

	  // get calibrations for phonons and charge
	  int dettype = pulseCollection[jChan].GetDetectorType();
	  double ptCalibration = myUserData.GetOverallPTCalibrationTI(detnum,dettype);
	  double chanTypeCalibration;
	  if(pulseCollection[jChan].GetChannelName()[0] == 'P')
	      chanTypeCalibration = myUserData.GetOverallPTCalibrationTI(detnum, dettype);
	  else if(pulseCollection[jChan].GetChannelName()[0] == 'Q')
	      chanTypeCalibration = myUserData.GetQCalibration(detnum, dettype, pulseCollection[jChan].GetChannelName());

        // declare the branch pointer
	std::vector<double> channelPulse = pulseCollection[jChan].GetBaselineSubNormPulse();
	string channelName = pulseCollection[jChan].GetChannelName();

	// normalize the pulse
	channelPulse = PulseTools::Normalize(channelPulse, ptnorm * ptCalibration / chanTypeCalibration);

	chanStr.push_back(channelName);

	if(channelName == "PT")
	    chanAmp.push_back(pulseCollection[jChan].GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFamps"));
	else if(channelName[0] == 'P')
	    chanAmp.push_back(pulseCollection[jChan].GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps"));
	else if(channelName[0] == 'Q')
	    chanAmp.push_back(pulseCollection[jChan].GetPulseAnalysis("OptimalFilterCharge2X2").GetRQVal("OFvolts"));

	// fill the traces
	traceMap[detnum][channelName].clear();   // clear out vector before filling
	traceMap[detnum][channelName] = channelPulse;
	//std::cout << channelName << " trace size = " << traceMap[detnum][channelName].size() << std::endl;

	// assign the vectors
	/*if(channelName == "PAS1")
	    h_PAS1 = channelPulse;
	else if(channelName == "PBS1")
            h_PBS1 = channelPulse;
	else if(channelName == "PCS1")
            h_PCS1 = channelPulse;
	else if(channelName == "PDS1")
            h_PDS1 = channelPulse;
	else if(channelName == "PAS2")
            h_PAS2 = channelPulse;
        else if(channelName == "PBS2")
            h_PBS2 = channelPulse;
        else if(channelName == "PCS2")
            h_PCS2 = channelPulse;
        else if(channelName == "PDS2")
            h_PDS2 = channelPulse;
	else if(channelName == "QIS1")
            h_QIS1 = channelPulse;
	else if(channelName == "QOS1")
            h_QOS1 = channelPulse;
	else if(channelName == "QIS2")
            h_QIS2 = channelPulse;
	else if(channelName == "QOS2")
            h_QOS2 = channelPulse;
	else if(channelName == "PT")
            h_PT = channelPulse;
	else if(channelName == "PS1")
            h_PS1 = channelPulse;
        else if(channelName == "PS2")
	h_PS2 = channelPulse;*/
      }

      // write the tree
      //treeVector[detnum-1]->Fill();
      //treeMap[detnum]->Print("all");
      treeMap[detnum]->Fill();

      // clear all the vectors
      /*h_PAS1.clear();
      h_PBS1.clear();
      h_PCS1.clear();
      h_PDS1.clear();
      h_PAS2.clear();
      h_PBS2.clear();
      h_PCS2.clear();
      h_PDS2.clear();
      h_QIS1.clear();
      h_QOS1.clear();
      h_QIS2.clear();
      h_QOS2.clear();
      h_PT.clear();
      h_PS1.clear();
      h_PS2.clear();*/
      //traceMap.clear();
      chanStr.clear();
      chanAmp.clear();
    }

    // eat the asterisk and the rest of the lin
    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    jevt++;
  }  

  for(map<int, TTree*>::iterator it = treeMap.begin(); it != treeMap.end(); it++)
  {
      it->second->Write();
      it->second->Print("all");
  }

  templateFile.Write();
  templateFile.Print();
  templateFile.Close();

  return 0;

} //end main()   DONE!!!



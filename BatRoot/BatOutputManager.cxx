///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: BatOutputManager
//Authors: L. Hsu, B. Serfass
//Description:  This class controls the generation of the output files.   It defines and manages 
//a series of maps which contain the RQ's and their values.   It stores the RQ values in ROOT TTree objects.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "TTree.h"

#include "BatOutputManager.h"
#include "PulseData.h"
#include "ChannelMapHelper.h"

//User Analysis Classes - DO NOT modify or copy this comment (for auto_analysis)
#include "SimulateFromRandoms.h"
#include "OptimalFilterPhonon1X2.h"
#include "OptimalFilterPhononNS.h"
#include "OptimalFilterNxN.h"
#include "PSDIntegralPhonon.h"
#include "WedgeFitPhonon.h"
#include "ConstFreqRTFTWalkPhonon.h"
#include "F5ChargeX.h"
#include "InflectionTime.h"
#include "NoiseSelector.h"
#include "PulseIntegral.h"
#include "VarFreqRTFTWalkPhonon.h"
#include "RTFTWalkCharge.h"
#include "BasicPulseCalc.h"
#include "PipeFitPhonon.h"
#include "OptimalFilterPhonon.h"
#include "OptimalFilterChargeX.h"
#include "OptimalFilterCharge.h"
#include "VetoAnalysis.h"
#include "NoiseMonitorAnalysis.h"
#include "SingleExponentialFit.h"

#include <iomanip>
using namespace std;

// ======================================================================

BatOutputManager::BatOutputManager(UserDataManager& myUserData, DetectorConfigManager& myDetectorConfigManager,
				   const string& rawDataFilename)  :
   fUserData(myUserData),
   fEventTree(0),
   fVetoTree(0),
   fNoiseMonitorTree(0),
   fEventListLocked(true),
   fZipListLocked(true),
   fVetoListLocked(true),
   fNoiseMonitorListLocked(true),
   fDebugOn(false),
   fDetectorConfigManager(myDetectorConfigManager)
{
   //cout <<"Constructing BatOutputManager" << endl;  // constructor

   //Open output file
   string rqPath = fUserData.GetPath("RQ_DATA");
   string rqFilePrefix = myUserData.GetPrefix("RQ_DATA_PREFIX");
   fOutputFile = TFile::Open(Form("%s%s.root", rqPath.c_str(), (rqFilePrefix+rawDataFilename).c_str()), "recreate");
  
   // create directories
   fOutputFile->mkdir("infoDir","Info directory");   
   fOutputFile->mkdir("detectorConfigDir","Detector configuration directory");   
   fOutputFile->mkdir("rqDir","RQ directory");  
  
   // RQ structure
   fOutputFile->cd("rqDir");

   // Event tree
   fEventTree = new TTree("eventTree","eventTree");

   // Veto tree
   if(fUserData.DoVetoProcessing() && fUserData.WriteVetoRQ()) fVetoTree = new TTree("vetoTree","vetoTree");

   //Noise monitor tree
   if(fUserData.DoNoiseMonitorProcessing() && fUserData.WriteNoiseMonitorRQ())
     fNoiseMonitorTree = new TTree("noiseMonitorTree","noiseMonitorTree");
   
   // Zip tree
   fDetectorMap = fDetectorConfigManager.GetDetectorMap();
   map< int, int >::iterator it;

   for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
    {
        
      int detNum = it->first;
      
      if(fUserData.WriteZipRQ(detNum))
      {
	 if(fDebugOn) cout <<"making a new tree!" << detNum << endl;

	 //Important! tree name = "zip#", tree title = "#"
	 //the tree title is used by BatOutputMan to match the tree with the rq's of the corresponding zip, 
	 //so don't modify this unless you know what you are doing!!
	 fZipTreeVector.push_back(new TTree(Form("zip%d",detNum), Form("%d", detNum))); 
	 
	 //next, reserve a slot for a zip RQ map
	 map<string, double> dummyMap;
	 fMapOfZipMaps.insert(pair<int, map<string, double> >(detNum, dummyMap));
      }
   }
}

  
BatOutputManager::~BatOutputManager() 
{ 
   //cout <<"Goodbye from BatOutputManager()" << endl; // destructor
}

//Reinitialize lists to default values
void BatOutputManager::ResetLists()
{
   double initval =  -999999.;

   if(fDebugOn) cout <<"BatOutputManager::Resetting output list values! " << endl;

   //reset event list
   map<string,double>::iterator eventListItr = fEventListMap.begin();
   for( ; eventListItr!=fEventListMap.end(); eventListItr++)
   {
      eventListItr->second = initval;
   }

   //reset veto list
   map<string,double>::iterator vetoListItr = fVetoListMap.begin();
   for( ; vetoListItr!=fVetoListMap.end(); vetoListItr++)
   {
      vetoListItr->second = initval;
   }

   //reset noise monitor list
   map<string,double>::iterator nmListItr = fNoiseMonitorListMap.begin();
   for( ; nmListItr!=fNoiseMonitorListMap.end(); nmListItr++)
   {
      nmListItr->second = initval;
   }

   
   //reset zip map, loop over the map of maps
   map< int, map<string,double> >::iterator zipItr = fMapOfZipMaps.begin();
   for( ; zipItr != fMapOfZipMaps.end(); zipItr++)
   {
     //now loop over zip RQ map
     map<string,double>::iterator zipMapItr = (zipItr->second).begin();
     for( ; zipMapItr != (zipItr->second).end(); zipMapItr++)
      zipMapItr->second = initval;
   }

   return;
}

// ======================= Construct Output ==================================

void BatOutputManager::ConstructOutputLists()
{

   ConstructEventOutputList();
   ConstructZipOutputList();
   if(fVetoTree != 0) ConstructVetoOutputList();  //don't construct this list if tree is not initialized
   if(fNoiseMonitorTree) ConstructNoiseMonitorOutputList();

   //Call when you are finished making the lists!
   ConfigureOutputTrees();
   
   return;
}

void BatOutputManager::ConstructEventOutputList()
{
   //unlock list
   fEventListLocked = false;

   //====== Variables for the event tree ======

   //from Event Header - this is one way to declare an RQ for writeout
   AddEventListVar("EventType");
   AddEventListVar("EventCategory");
   
   //from Admin - this is another (more automated way) to declare RQ's for writeout
   AdminData emptyAdminData;  //create empty AdminData for querying analysis rq's
   emptyAdminData.ConstructRQList(); //constucts list internally in class
   map<string, double> adminRQList = emptyAdminData.GetRQList();  
   AddEventListVar(adminRQList);

   //from External
   GPSData emptyGPSData;           //create empty GPSData for querying analysis rq's
   emptyGPSData.ConstructRQList(); //constucts list internally in class
   map<string, double> externalRQList = emptyGPSData.GetRQList();  
   AddEventListVar(externalRQList);

   if(fUserData.DoTriggerProcessing() && fUserData.WriteTriggerRQ())
   {
      //from History
      HistoryData emptyHistoryData;  //create empty HistoryData for querying analysis rq's
      emptyHistoryData.ConstructRQList(fUserData.GetIntParameter("MAX_TOWERS")); //constructs list internally in class
      map<string, double> historyRQList = emptyHistoryData.GetRQList();  
      AddEventListVar(historyRQList);
      
      //from Trigger
      TriggerData emptyTriggerData;  //create empty TriggerData for querying analysis rq's
      emptyTriggerData.ConstructRQList(fUserData.GetIntParameter("MAX_TOWERS")); //constructs list internally in class
      map<string, double> triggerRQList = emptyTriggerData.GetRQList();  
      AddEventListVar(triggerRQList);
   }


   //from GPIB file
   if (fUserData.DoRead("GPIB_FILE")) {
       GpibDataManager emptyGpibData;  //create empty GpibData for querying RQ's
       map<string, double> gpibRQList = emptyGpibData.GetRQList();  
       AddEventListVar(gpibRQList);
    }
   
   // from ISR file (LastISRTime) 
   // (Note that the other quantities from ISR file, bias and biastime, are stored in the ZIP Trees)   
   if (fUserData.DoRead("ISR_FILE")) {
     IsrDataManager emptyIsrData(fUserData.GetMaxZIPs()); //create empty GpibData for querying RQ's
     map<string, double> isrRQList = emptyIsrData.GetEventRQList();  
     AddEventListVar(isrRQList);
   }
    
   //from database
   if(fUserData.DoRead("DATABASE") ){
     CdmsDB::DatabaseManager emptyDbMan;
     AddEventListVar(emptyDbMan.ConstructEventRQs());
   }
    
   //locking list
   fEventListLocked = true; 

   return;
}

void BatOutputManager::ConstructVetoOutputList()
{
   //unlock list
   fVetoListLocked = false;

   //====== Variables for the veto tree ======

   //panel-by-panel Veto RQ's
   VetoAnalysis emptyVetoAnalysis;  //create empty VetoAnalysis for querying rq's
   map<string,double> vetoRQList = emptyVetoAnalysis.GetRQList(); 
   AddVetoListVar(vetoRQList);
   
   if(fDebugOn) 
   cout <<"Adding veto RQ list to BatOutput!!!!!!!!!!!" << endl;

   //locking list
   fVetoListLocked = true; 

   return;
}

void BatOutputManager::ConstructNoiseMonitorOutputList()
{
   //unlock list
   fNoiseMonitorListLocked = false;

   //====== Variables for the veto tree ======

   //panel-by-panel NoiseMonitor RQ's
   NoiseMonitorAnalysis emptyNoiseMonitorAnalysis;  //create empty NoiseMonitorAnalysis for querying rq's
   map<string,double> nmRQList = emptyNoiseMonitorAnalysis.GetRQList(); 
   AddNoiseMonitorListVar(nmRQList);
   
   if(fDebugOn) 
   cout <<"Adding veto RQ list to BatOutput!!!!!!!!!!!" << endl;

   //locking list
   fNoiseMonitorListLocked = true; 

   return;
}


void BatOutputManager::ConstructZipOutputList()
{
   //unlock list
   fZipListLocked = false;

   //====== Variables for the zip trees ======

   //Create an empty PulseData for querying analysis rq's
   //PulseData emptyPulseData;

   // Create empty DMM/ISR data for querying RQ's
   DmmDataManager emptyDmmData(fDetectorMap);  
   
   // Create empty UserDataManager for querying RQ's
   UserDataManager emptyUserData;
   emptyUserData.ConstructZipRQList(fDetectorMap);


   // ============ query for RQ list of given analysis class ==============
   //allchan = save for all channel on a zip
   //phononchan = save for each phonon channel 
   //phononchantotal = save for each phonon channel and PT (sum of all phonon channels)
   //chargechan = save for each charge channel
   //chargechantotal = save for each charge channel and PT (sum of all charge channels)
   //no flag = save only once per zip


   //loop over zip list and store RQ's based on analysis + external files list

   map< int, map<string,double> >::const_iterator mapItr = fMapOfZipMaps.begin();
   for( ; mapItr != fMapOfZipMaps.end(); mapItr++)
   {
     //for each zip, get the analysis lists.
     //Then retrieve the RQ lists from each analysis class
     int detNum = mapItr->first;
     int detType = fDetectorMap[detNum];

     //store these RQs only once per zip
     AddZipListVar("Empty", detNum);  //special RQ to flag selectively read zips
     AddZipListVar("DetType",detNum); //special RQ to store detector type

     //Get the list of analyses for this zip from ExtData

     // loop over phonon analysis classes
     // make a list of all possibilities first then only add vars for the ones activated
     // inside the loop. this allows for you to have a PSIDES or PT algorithm without having
     // an individual phonon version [ANV] 
     list<string> phononAlgorithms = fUserData.GetListAlgorithms(detNum,"phonon"); // only enable algorithms + BasicPulseCalc + SimulateFromRandoms (if chosen)
     list<string> PTAlgorithms = fUserData.GetListAlgorithms(detNum,"PT"); 
     list<string> PSIDESAlgorithms = fUserData.GetListAlgorithms(detNum,"PSIDES"); 
     list<string>::iterator phononEndItr = phononAlgorithms.end();
     phononAlgorithms.splice(phononEndItr,PTAlgorithms);
     phononEndItr = phononAlgorithms.end();
     phononAlgorithms.splice(phononEndItr,PSIDESAlgorithms);
     phononAlgorithms.sort();    //unique() only removes neighboring redundancy
     phononAlgorithms.unique();  //remove redundancy which causes error in AddZipListVar(..)
     list<string>::const_iterator phononListItr = phononAlgorithms.begin();
     for( ; phononListItr != phononAlgorithms.end(); phononListItr++)
     {
        if(fDebugOn) cout << "Add RQs from Algorithm: " << *phononListItr << endl;
        map<string,double> analysisRQList = GetRQList(*phononListItr, "phonon"); //creates dummy instance of analysis class to get RQList
   
          

 
        // RQ for each sensor
        if (fUserData.DoAlgorithm(detNum,"phonon",*phononListItr)){
                   AddZipListVar(analysisRQList, detNum, "phononchan"); 
        }
        // add total phonon (need to check if it is turn on because the list of 
        if (fUserData.DoAlgorithm(detNum,"PT",*phononListItr)){
                   AddZipListVar(analysisRQList, detNum, "phonontotal");
        } 
        // add sum of channels for each side (iZIP)
        if (fUserData.DoAlgorithm(detNum,"PSIDES",*phononListItr)){
                   AddZipListVar(analysisRQList, detNum, "phononsides");
        } 
        // MUST have the basic pulse algorithms
        if(*phononListItr == "BasicPulseCalc"){
                   AddZipListVar(analysisRQList, detNum, "phononchan");
	}

        // add the sim pulse algorithms
	if(*phononListItr == "SimulateFromRandoms" && fUserData.GetIntParameter("DO_PTSIM")==1 && 
	   (fUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1 || fUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)){ 
	  AddZipListVar(analysisRQList, detNum, "phonontotal");
        }

 	if(*phononListItr == "SimulateFromRandoms" && fUserData.GetIntParameter("DO_PHONONSIM")==1 && 
	   (fUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1 || fUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)){ 
 	  AddZipListVar(analysisRQList, detNum, "phononchan");
         }

 	if(*phononListItr == "SimulateFromRandoms" && fUserData.GetIntParameter("DO_PSIDESSIM")==1 && 
	   (fUserData.GetIntParameter("DO_SIM_FROM_TEMPLATE")==1 || fUserData.GetIntParameter("DO_SIM_FROM_PULSE")==1)){ 
 	  AddZipListVar(analysisRQList, detNum, "phononsides");
         }


       // if(fDebugOn)cout <<"Adding class: " << *phononListItr <<", to zip" << detNum << endl;
      
     }   

     // loop over charge analysis classes
     list<string> chargeAlgorithms = fUserData.GetListAlgorithms(detNum, "charge");
     list<string>::const_iterator chargeListItr = chargeAlgorithms.begin();
     for( ; chargeListItr != chargeAlgorithms.end(); chargeListItr++)
     {
       map<string,double> analysisRQList = GetRQList(*chargeListItr, "charge"); //creates dummy instance of analysis class to get RQList

       if(*chargeListItr != "SimulateFromRandoms")
       {
	 AddZipListVar(analysisRQList, detNum, "chargechan");

	 if(fDebugOn) 
	   cout <<"Adding class: " << *chargeListItr <<", to zip" << detNum << endl;

       }
       else
       {  
	 if(fUserData.GetIntParameter("DO_CHARGESIM")==1)
	   {
	     AddZipListVar(analysisRQList, detNum, "chargechan");

	     if(fDebugOn)
	       cout <<"Adding class: " << *chargeListItr <<", to zip" << detNum << endl;
	   }
       }

     } //end loop over charge algorithms

     //Get list from DMM file
     if (fUserData.DoRead("DMM_FILE")) {
       map<string, double> dmmRQList = emptyDmmData.GetZipRQList(detNum); 
       AddZipListVar(dmmRQList, detNum);
     }
   
     //Get list Detector RQ  file
     if (fUserData.DoRead("DET_STATUS_FILE")) {
       map<string, double> detstatusRQList = emptyUserData.GetZipRQList(detNum); 
       AddZipListVar(detstatusRQList, detNum);
     }
     
     //Get list from Database
     if( fUserData.DoRead("DATABASE")) {
       CdmsDB::DatabaseManager emptyDbMan;
       AddZipListVar(emptyDbMan.ConstructPulseRQs("phonon"), 
		     detNum,"phononchan");
       AddZipListVar(emptyDbMan.ConstructPulseRQs("charge") ,
		     detNum,"chargechan");
       AddZipListVar(emptyDbMan.ConstructDetectorRQs(detType,detNum), 
		     detNum,"");
       
     }

   } //end loop over zip list
   
   //locking list
   fZipListLocked = true; 

   //suffixes (QI, QO, PA-PD) are fixed in pulsedata - get these from config or set as constants?

   return;
}


// ======================= Store Output ==================================


void BatOutputManager::StoreOutput(const EventBuilder& eventBuilder) 
{
   StoreEventOutput(eventBuilder);
   StoreZipOutput(eventBuilder);
   if(fVetoTree != 0) StoreVetoOutput(eventBuilder); //only store if veto tree has been initialized
   if(fNoiseMonitorTree) StoreNoiseMonitorOutput(eventBuilder);

   //Fill the trees & reset values in the list!
   FillTrees();
   ResetLists();

   // Reset RQ list inside external files managers
    eventBuilder.GetGpibData().ResetRQList();
    eventBuilder.GetDmmData().ResetRQList();
    eventBuilder.GetIsrData().ResetRQList();


   return;
}

void BatOutputManager::StoreEventOutput(const EventBuilder& eventBuilder) 
{
   //====== Store event tree variables =======

   //from the EventHeader (stored in EventBuilder because there's no EventHeader class)
   //this is one way to store RQs in the output
   SetEventListVal("EventType", eventBuilder.GetEventType());
   SetEventListVal("EventCategory", eventBuilder.GetEventCategory());

   //from the Admin Record
   //this is another (more automated) way to store RQs in the output
   map<string, double> adminRQList = eventBuilder.GetAdmin().GetRQList();
   SetEventListVal(adminRQList);

   //from the External Record
   map<string, double> externalRQList = eventBuilder.GetGPS().GetRQList();
   SetEventListVal(externalRQList);

   if(fUserData.DoTriggerProcessing() && fUserData.WriteTriggerRQ())
   {
      //from the History Record
      map<string, double> historyRQList = eventBuilder.GetHistory().GetRQList();
      SetEventListVal(historyRQList);
      
      //from the Trigger Record
      map<string, double> triggerRQList = eventBuilder.GetTrigger().GetRQList();
      SetEventListVal(triggerRQList);
   }

   //from GPIB file 
   if (fUserData.DoRead("GPIB_FILE")) {
      map<string, double> gpibRQList = eventBuilder.GetGpibData().GetRQList();
      SetEventListVal(gpibRQList);
   }

   //from ISR file 
   if (fUserData.DoRead("ISR_FILE")) {
      map<string, double> isrRQList = eventBuilder.GetIsrData().GetEventRQList();
      SetEventListVal(isrRQList);
   }


   //from database
   if(fUserData.DoRead("DATABASE") ){
     SetEventListVal(eventBuilder.GetDatabaseManager().GetEventRQList());
   }
   return;
}

//similar but simpler than StoreZipOutput
void BatOutputManager::StoreVetoOutput(const EventBuilder& eventBuilder) 
{
   //====== Store veto tree variables =======

   vector<PulseData> pulseCollection;
   eventBuilder.FillVetoPulseCollection(pulseCollection);

   for(uint pulseItr=0; pulseItr < pulseCollection.size(); pulseItr++)
   {
      PulseData aPulseData = pulseCollection[pulseItr]; 
      int panelNum = aPulseData.GetDetectorNum();
      
      // ======== Iterate over list of analysis classes and store the RQs  ================
      
      vector<TCDMSAnalysis> analysisCollection = aPulseData.GetAnalysisCollection(); //only one VetoAnalysis right now!
      
      for(uint anaItr=0; anaItr < analysisCollection.size(); anaItr++)
      {
	 map<string,double> rqList = analysisCollection[anaItr].GetRQList(); 
	 SetVetoListVal(rqList, panelNum);
	 
	 if(fDebugOn) 
	    cout <<"Storing rq's for veto analysis class: " << analysisCollection[anaItr].GetClassName() << endl;
      }
      
      if(fDebugOn)
	 cout <<"Panel Num = " << panelNum << endl;
      
   } //end loop over veto pulses

   return;
}

void BatOutputManager::StoreNoiseMonitorOutput(const EventBuilder& eventBuilder) 
{
   //====== Store veto tree variables =======

   vector<PulseData> pulseCollection;
   eventBuilder.FillNoiseMonitorPulseCollection(pulseCollection);

   for(uint pulseItr=0; pulseItr < pulseCollection.size(); pulseItr++)
   {
      PulseData aPulseData = pulseCollection[pulseItr]; 
      string chName = aPulseData.GetChannelName();
      if(!fUserData.WriteNoiseMonitorRQ(chName))
	continue;
      // ======== Iterate over list of analysis classes and store the RQs  ================
      vector<TCDMSAnalysis> analysisCollection = aPulseData.GetAnalysisCollection(); //only one NoiseMonitorAnalysis right now!
      
      for(uint anaItr=0; anaItr < analysisCollection.size(); anaItr++)
      {
	 map<string,double> rqList = analysisCollection[anaItr].GetRQList(); 
	 SetNoiseMonitorListVal(rqList, chName);
	 
	 if(fDebugOn) 
	    cout <<"Storing rq's for noise monitor analysis class: " << analysisCollection[anaItr].GetClassName() << endl;
      }
      
      if(fDebugOn)
	 cout <<"Channel Name = " << chName << endl;
      
   } //end loop over noise monitor pulses

   return;
}

void BatOutputManager::StoreZipOutput(const EventBuilder& eventBuilder) 
{

   //====== Store zip tree variables =======

   //Get DMM RQ lists
    map<int, map<string,double> > dmmRQList;
    if (fUserData.DoRead("DMM_FILE"))
      dmmRQList = eventBuilder.GetDmmData().GetAllZipRQList();

    //Get Detector RQ lists
    map<int, map<string,double> > detstatusRQList;
    if (fUserData.DoRead("DET_STATUS_FILE"))
      detstatusRQList = fUserData.GetAllZipRQList();

   //loop over zips w/ trees and get the pulse collection for each zip
 
   for(uint zipItr = 0; zipItr < fZipTreeVector.size(); zipItr++)
   {
     vector<PulseData> pulseCollection;
     int zipNum = atoi(fZipTreeVector[zipItr]->GetTitle());
     eventBuilder.FillSingleZipPulseCollection(pulseCollection, zipNum);

     //save these variables once per zip
     if(pulseCollection.size() != 0) SetZipListVal("Empty", "", 0, zipNum); //special RQ to flag selectively read zips
     
     //loop over pulses for this zip and store the values, if no pulses than move to next zip
     for(uint pulseItr=0; pulseItr < pulseCollection.size(); pulseItr++)
     {
       PulseData aPulseData = pulseCollection[pulseItr]; 
       int detNum = aPulseData.GetDetectorNum();
       string chanName = aPulseData.GetChannelName();
     	
       // ======== Iterate over list of analysis classes and store the RQs  ================

       vector<TCDMSAnalysis> analysisCollection = aPulseData.GetAnalysisCollection(); //probably very slow!
       
       for(uint anaItr=0; anaItr < analysisCollection.size(); anaItr++)
       {
	  map<string,double> rqList = analysisCollection[anaItr].GetRQList(); 
	  SetZipListVal(rqList, chanName, detNum);

	  if(fDebugOn) cout <<"Storing rq's for analysis class: " << analysisCollection[anaItr].GetClassName() << endl;
       }
       
       // ======== Store the following only once per detector ================

       SetZipListVal("DetType", "", aPulseData.GetDetectorType(), detNum); //special RQ to store detector type (overwrites for multiple channels)

       if(fDebugOn)
	 cout <<"Detector Num = " << aPulseData.GetDetectorNum()
	      <<"\nChannel Num = " << aPulseData.GetDetectorChannel() << endl;

     } //end loop over pulses on this zip

        
     // Store list from DMM/ISR file
      if (fUserData.DoRead("DMM_FILE"))
        {  
           map<string,double> rqList = (dmmRQList.find(zipNum))->second;
           SetZipListVal(rqList, "", zipNum);
        }

      // Store detector status RQs
      if (fUserData.DoRead("DET_STATUS_FILE"))
        {  
	  map<string,double> rqList = (detstatusRQList.find(zipNum))->second;
	  SetZipListVal(rqList, "", zipNum);
        }
      
      //store database list
      if (fUserData.DoRead("DATABASE")){
	map<string, double> rqList = 
	  eventBuilder.GetDatabaseManager().GetDetectorRQList(zipNum);
	if(!rqList.empty())
	  SetZipListVal(rqList,"",zipNum);
      }

        
   } //end loop over zips

   return;
}

// ==================== Tree Management routines ===================================


// Set Branch addresses here 
void BatOutputManager::ConfigureOutputTrees()
{

   // ==== formatting event tree ====
 
   map<string,double>::iterator eventListItr = fEventListMap.begin();
   for( ; eventListItr!=fEventListMap.end(); eventListItr++)
   {
      string name = eventListItr->first;
      if(fDebugOn) cout <<"Adding Branch: " << name+"/D" <<", w/ initial value= " << eventListItr->second << endl;
      fEventTree->Branch(name.c_str(), &(eventListItr->second), (name+"/D").c_str());
   }

   // ==== formatting veto tree ====

   //only format if tree is initialized
   if(fVetoTree != 0)
   {
      map<string,double>::iterator vetoListItr = fVetoListMap.begin();
      for( ; vetoListItr!=fVetoListMap.end(); vetoListItr++)
      {
	 string name = vetoListItr->first;
	 if(fDebugOn) 
	    cout <<"Adding Branch: " << name+"/D" <<", w/ initial value= " << vetoListItr->second << endl;
	 fVetoTree->Branch(name.c_str(), &(vetoListItr->second), (name+"/D").c_str());
      }
   }

   // ==== formatting noise monitor tree ====

   //only format if tree is initialized
   if(fNoiseMonitorTree != 0)
   {
      map<string,double>::iterator nmListItr = fNoiseMonitorListMap.begin();
      for( ; nmListItr!=fNoiseMonitorListMap.end(); nmListItr++)
      {
	 string name = nmListItr->first;
	 if(fDebugOn) 
	    cout <<"Adding Branch: " << name+"/D" <<", w/ initial value= " << nmListItr->second << endl;
	 fNoiseMonitorTree->Branch(name.c_str(), &(nmListItr->second), (name+"/D").c_str());
      }
   }

   
   // ==== formatting zip trees ====

   //loop over map of maps
   map< int, map<string,double> >::iterator zipMap = fMapOfZipMaps.begin();
   for( ; zipMap != fMapOfZipMaps.end(); zipMap++)
   {
     //now loop over zip RQ map
     map<string,double>::iterator zipMapItr = (zipMap->second).begin();
     for( ; zipMapItr != (zipMap->second).end(); zipMapItr++)
     {
       //Format of name should be zip#_paramName
       string name = zipMapItr->first;
       int    rqZipNum =  atoi(name.c_str());
       size_t breakPoint = name.find_first_of("_");
       string subname = name.substr(breakPoint+1, name.size());
       
       //loop over the zipTreeVector to find the tree that matches the zip number
       for(uint vitr = 0; vitr < fZipTreeVector.size(); vitr++)
       {   
	 int treeZipNum = atoi(fZipTreeVector[vitr]->GetTitle());
	   
	 //found the corresponding tree
	 if(rqZipNum == treeZipNum)
	 {
	   fZipTreeVector[vitr]->Branch(subname.c_str(), &(zipMapItr->second), (subname+"/D").c_str());

	   if(fDebugOn)
	      cout <<"Testing string manipulations! " 
		   <<"\nAdding Branch: " << subname+"/D, " <<"For detector: " << rqZipNum <<", w/ initial value= " << zipMapItr->second 
		   <<"\nfor tree: " << treeZipNum<< endl;
	 }

       } //end loop over vector of trees
     }//end loop over ziplist
   } //end loop over map of maps

   return;
}

void BatOutputManager::FillTrees()
{
   // fill event tree
   fEventTree->Fill();

   // fill the veto tree
   if(fVetoTree != 0) fVetoTree->Fill();  //don't fill if not initialized

   // fill the noise monitor tree
   if(fNoiseMonitorTree != 0) fNoiseMonitorTree->Fill();  //don't fill if not initialized

   // fill zip trees
   for(uint zipItr = 0; zipItr < fZipTreeVector.size(); zipItr++)
   {
     fZipTreeVector[zipItr]->Fill();
   }

   return;
}

void BatOutputManager::WriteTrees()
{
   // RQ structure
   fOutputFile->cd("rqDir");

   // event tree
   fEventTree->Write();
   
   // veto tree
   if(fVetoTree != 0) 
   {
      fVetoTree->Write();
   }

   // noise monitor tree
   if(fNoiseMonitorTree != 0) 
   {
      fNoiseMonitorTree->Write();
   }

   // zip tree
   for(uint zipItr = 0; zipItr < fZipTreeVector.size(); zipItr++)
   {
     fZipTreeVector[zipItr]->Write();
   }


   //Done writing!
   cout <<"Done writing output RQ file: " << fOutputFile->GetName() << endl;
   fOutputFile->Close();

   return;
} 
     


void  BatOutputManager::CreateAndWriteSettingsTree()
{
  
 // === create the config tree === 

 fOutputFile->cd("infoDir");
 TTree* configTree = new TTree("userSettingsTree","userSettingsTree");


 // ==== double paramameters branches ====

 map<string,vector<double> > rqDoubleListMap = fUserData.GetVectDoubleRQList();
 map<string,vector<double> >::iterator configDoubleListItr = rqDoubleListMap.begin();
 for( ; configDoubleListItr!=rqDoubleListMap.end(); configDoubleListItr++)
     {
         int sizeVect = configDoubleListItr->second.size();
         string name = configDoubleListItr->first;
         configTree->Branch(name.c_str(), &(configDoubleListItr->second)[0], Form("%s[%d]/D",name.c_str(),sizeVect));
       
     }

//  ==== integer paramameters  branches ====

 map<string,vector<int> > rqIntListMap = fUserData.GetVectIntRQList();
 map<string,vector<int> >::iterator configIntListItr = rqIntListMap.begin();
 for( ; configIntListItr!=rqIntListMap.end(); configIntListItr++)
     {
         int sizeVect = configIntListItr->second.size();
         string name = configIntListItr->first;
         configTree->Branch(name.c_str(), &(configIntListItr->second)[0], Form("%s[%d]/I",name.c_str(),sizeVect));
       
     }

 // ====  Fill and write ====

 configTree->Fill();
 configTree->Write();

}

void  BatOutputManager::CreateAndWriteDetectorConfigTree()
{

  // loop over detectors
  fDetectorMap = fDetectorConfigManager.GetDetectorMap();
  map< int, int >::iterator it;

  for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
  {

    // === Create a tree and store configuration for each detector ===
    int detNum = it->first;

    fOutputFile->cd("detectorConfigDir");
    TTree* configTree = new TTree(Form("detectorConfigZip%d", detNum), Form("detectorConfigZip%d", detNum));

    // store the detector tower index, which actually comes from the user settings file
    double detTowerIndex = (double)fUserData.GetIntParameter(detNum, "INDEX_IN_TOWER");
    configTree->Branch("IndexWithinTower", &detTowerIndex, "IndexWithinTower/D");

    // now loop over the detector config map and store all quantities
    map<string, double> detConfigTest = fDetectorConfigManager.GetDetectorConfiguration(detNum);    
    map< string, double >::iterator detConfigItr = detConfigTest.begin();
    
    for( ; detConfigItr != detConfigTest.end(); detConfigItr++)
    {

      string name = detConfigItr->first;
      
      //cout <<"Adding Branch: " << name+"/D" <<", w/ value= " << detConfigItr->second << endl;
      
      configTree->Branch(name.c_str(), &(detConfigItr->second), (name+"/D").c_str());

    } // end loop over config parameters


    // === Fill and write ===
    configTree->Fill();
    configTree->Write();

  } // end loop over detectors

}


void  BatOutputManager::CreateAndWriteFilterTree(FilterDataManager& myFilterData)
{

  // ==== create filter tree in infoDir (one per detector) ===
  fOutputFile->cd("infoDir");

  // ==== loop detectors ===

  map< int, int >::iterator it;
  for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
   {
    int detNum = it->first;
     
    // == create tree ==
    TTree *filterTree = new TTree(Form("filterTreeZip%d",detNum),Form("Z%d filter information tree",detNum));

    // == get RQ informations ==
   
    //loop double map
    map<string,double> zipDoubleRQmap = myFilterData.GetDoubleRQList(detNum);
    map<string,double>::iterator zipDoubleRQmapItr = zipDoubleRQmap.begin();
    for( ; zipDoubleRQmapItr!=zipDoubleRQmap.end(); zipDoubleRQmapItr++)
     {
         string name = zipDoubleRQmapItr->first;
         filterTree->Branch(name.c_str(), &(zipDoubleRQmapItr->second), Form("%s/D",name.c_str()));
       
     }
 
    // string map FIXME (need to be automcatic)
    map<string,string> zipStringRQmap = myFilterData.GetStringRQList(detNum);
     
    // template tag
    string templateTagStr =zipStringRQmap.find("templateTag")->second;
    char templateTag[templateTagStr.size()+1];
    strcpy(templateTag,templateTagStr.c_str()); 
    filterTree->Branch("templateTag", &templateTag,"templateTag/C");
   
    // filter tag
    string filterTagStr =zipStringRQmap.find("filterTag")->second;
    char filterTag[filterTagStr.size()+1];
    strcpy(filterTag,filterTagStr.c_str()); 
    filterTree->Branch("filterTag", &filterTag,"filterTag/C");
   
    // filter production date
    string filterProductionDateStr =zipStringRQmap.find("filterProductionDate")->second;
    char filterProductionDate[filterProductionDateStr.size()+1];
    strcpy(filterProductionDate,filterProductionDateStr.c_str()); 
    filterTree->Branch("filterProductionDate", &filterProductionDate,"filterProductionDate/C");
   
    // noiseCodeGitTag
    string noiseCodeGitTagStr_cdmsbats =zipStringRQmap.find("noiseCodeGitTag_cdmsbats")->second;
    char noiseCodeGitTag_cdmsbats[noiseCodeGitTagStr_cdmsbats.size()+1];
    strcpy(noiseCodeGitTag_cdmsbats,noiseCodeGitTagStr_cdmsbats.c_str()); 
    filterTree->Branch("noiseCodeGitTag_cdmsbats", &noiseCodeGitTag_cdmsbats,"noiseCodeGitTag_cdmsbats/C");
    string noiseCodeGitTagStr_batcommon =zipStringRQmap.find("noiseCodeGitTag_batcommon")->second;
    char noiseCodeGitTag_batcommon[noiseCodeGitTagStr_batcommon.size()+1];
    strcpy(noiseCodeGitTag_batcommon,noiseCodeGitTagStr_batcommon.c_str()); 
    filterTree->Branch("noiseCodeGitTag_batcommon", &noiseCodeGitTag_batcommon,"noiseCodeGitTag_batcommon/C");
   

    // === fill/write tree ===
    filterTree->Fill();
    filterTree->Write(); 
  }    
}



void  BatOutputManager::CreateAndWriteProcessingInfoTree(string date, string gitTag_cdmsbats, string gitTag_batcommon)
{

  // ==== create processing tree in infoDir (one per detector) ===
  fOutputFile->cd("infoDir");
  TTree *processingTree = new TTree("processingTree","Processing informations tree");

  // ==== convert informations inot char ====
  
  // date
  char dateChar[date.size()+1];
  strcpy(dateChar,date.c_str()); 

  // git tag
  char BatRootGitTag_cdmsbats[gitTag_cdmsbats.size()+1];
  strcpy(BatRootGitTag_cdmsbats,gitTag_cdmsbats.c_str()); 
  char BatRootGitTag_batcommon[gitTag_batcommon.size()+1];
  strcpy(BatRootGitTag_batcommon,gitTag_batcommon.c_str()); 

  // === make branches and fill/write tree
  processingTree->Branch("date", &dateChar, "date/C");
  processingTree->Branch("BatRootGitTag_cdmsbats", &BatRootGitTag_cdmsbats, "BatRootGitTag_cdmsbats/C");
  processingTree->Branch("BatRootGitTag_batcommon", &BatRootGitTag_batcommon, "BatRootGitTag_batcommon/C");
  processingTree->Fill();
  processingTree->Write(); 
  
}






// ================  For building the output lists ==============================

void BatOutputManager::AddEventListVar(const string& varName)
{
   double initval =  -999999.;

   //check that this is being called from the right place
   if(fEventListLocked) 
   {
      cout <<"BatOutputManager::AddEventListVar ERROR! Adding variables to event list outside of ConstructEventOutputList!"
	   <<"This is not allowed, check your code!" 
	   << endl;
      exit(1);
   }   

   //check that the variable does not exist yet
   if(fEventListMap.count(varName) > 0)
   {
      cout <<"BatOutputManager::AddEventListVar ERROR! Trying to add variable to event list, but it already exists! " << varName 
	   <<"\nCheck ConstructEventOutputList!" << endl;
      exit(1);
   }

   fEventListMap.insert(pair<string,double>(varName,initval));

   if(fDebugOn) cout <<"Adding event var!" << varName <<"= " << fEventListMap[varName] << endl;

   return;
}

void BatOutputManager::AddEventListVar(const map<string,double>& rqList)
{
   map<string, double>::const_iterator rqListItr = rqList.begin();
   
   for( ; rqListItr != rqList.end(); rqListItr++)   
   {  AddEventListVar(rqListItr->first);  }

   return;
}

void BatOutputManager::AddVetoListVar(const string& varName)
{
   double initval =  -999999.;

   //check that this is being called from the right place
   if(fVetoListLocked) 
   {
      cout <<"BatOutputManager::ERROR! Adding variables to veto list outside of ConstructVetoOutputList!"
	   <<"This is not allowed, check your code!" 
	   << endl;
      exit(1);

      return;
   }   

   //add variable for each veto panel
   uint panelMax = fUserData.GetIntParameter("MAX_VTPANELS");
   for(uint panelItr=1; panelItr <= panelMax; panelItr++)
   {
      string panelID = Form("%d",panelItr);
      if(panelID.length() < 2) panelID = "0" + panelID; //all veto prefixes have 2 digits
      string tempString = varName + panelID;

      //check that the variable does not exist yet
      if(fVetoListMap.count(tempString) > 0)
      {
	 cout <<"BatOutputManager::ERROR! Trying to add variable to veto list, but it already exists! " << varName 
	      <<"\nCheck ConstructVetoOutputList!" << endl;
	 exit(1);
      }

      fVetoListMap.insert(pair<string,double>(tempString,initval));

      if(fDebugOn) cout <<"Adding veto variable: " << tempString <<"= " << fVetoListMap[tempString] << endl;
   }

   return;
}

void BatOutputManager::AddVetoListVar(const map<string,double>& rqList)
{
   map<string, double>::const_iterator rqListItr = rqList.begin();
   
   for( ; rqListItr != rqList.end(); rqListItr++)   
   {  AddVetoListVar(rqListItr->first);  }

   return;
}


void BatOutputManager::AddNoiseMonitorListVar(const string& varName)
{
  double initval = BatRootTypes::kEmptyVariable;

   //check that this is being called from the right place
   if(fNoiseMonitorListLocked) 
   {
      cout <<"BatOutputManager::ERROR! Adding variables to noise mon list outside of ConstructNoiseMonitorOutputList!"
	   <<"This is not allowed, check your code!" 
	   << endl;
      exit(1);

      return;
   }   
   
   vector<string> detList = 
     fUserData.GetVectStringParameter("NOISEMONITOR_CHANNELS");

   for(size_t i=0; i<detList.size(); ++i){
     string fullName = detList[i] + varName;

     //check that the variable does not exist yet
     if(fNoiseMonitorListMap.count(fullName) > 0){
       cerr <<"BatOutputManager::ERROR! Trying to add variable to noisemon list, "
	    <<" but it already exists! " << varName 
	    <<"\nCheck ConstructNoiseMonitorOutputList!" << endl;
       exit(1);
     }
     
     fNoiseMonitorListMap.insert(pair<string,double>(fullName,initval));

     if(fDebugOn) cout <<"Adding noisemon variable: " << fullName
		       <<"= " << fNoiseMonitorListMap[fullName] << endl;
   
     
   }
}

void BatOutputManager::AddNoiseMonitorListVar(const map<string,double>& rqList)
{
   map<string, double>::const_iterator rqListItr = rqList.begin();
   
   for( ; rqListItr != rqList.end(); rqListItr++)   
   {  AddNoiseMonitorListVar(rqListItr->first);  }

   return;
}


//for adding a single RQ to a SINGLE zip
void BatOutputManager::AddZipListVar(const string& varName, const int zipNum, const string& multID)
{
   double initval =  -999999.;

   //check that this is being called from the right place
   if(fZipListLocked) 
   {
      cout <<"BatOutputManager::ERROR! Adding variables to zip list outside of ConstructZipOutputList!"
	   <<"This is not allowed, check your code!" 
	   << endl;
      exit(1);
      
      return;
   }   

   //retrive the map for this zip
   map< int, map<string,double> >::iterator zipMap = fMapOfZipMaps.find(zipNum);
   if(zipMap == fMapOfZipMaps.end())
   {
     cout <<"BatOutputManager::AddZipListVar ERROR!  Cannot find a map for specified detector in fMapOfZipMaps."
	  << endl;
     exit(1);
   }  
   

   // retrieve the detector type
   int detType = -999999;

   if(fDetectorMap.count(zipNum) > 0)
   {
     detType =  fDetectorMap[zipNum];
   }
   else
   {
     cout <<"BatOutputManager::AddZipListVar ERROR! Requested detector" << zipNum
	  <<" not found in map!"
	  << endl;
     exit(1);
   }

   //Loop over channels and add variable for each channel requested
   vector<string> chanList = GetChanList(multID, detType);
     
   for(uint chanItr=0; chanItr < chanList.size(); chanItr++)
   {

     string rootName = chanList[chanItr]+varName;
     string tempString = Form("%d_%s", zipNum, rootName.c_str());     
    
     //check that the variable does not exist yet
     if((zipMap->second).count(tempString) > 0)
     {
       cout <<"BatOutputManager::ERROR! Trying to add variable to zip list, but it already exists! " << rootName 
	    << endl;
       exit(1);
     }
       
     //add the RQ to the zip map for the chosen detector
     (zipMap->second).insert(pair<string,double>(tempString,initval));
     if(fDebugOn) 
       cout <<"Adding zip variable: " << tempString <<"= " << (zipMap->second)[tempString] << endl;
   
   } //done looping over channel list

   return;
}

//for adding a whole RQ list to a SINGLE zip
void BatOutputManager::AddZipListVar(const map<string,double>& rqList, const int zipNum, const string& multID)
{

   //Loop over list and call AddZipListVar for single RQs
   map<string,double>::const_iterator rqListItr = rqList.begin();
   for( ; rqListItr!=rqList.end(); rqListItr++)
   {
      //multID specifies whether RQ is created for each phonon channel or each charge channel, etc.
      string checkMultID = multID;

      //In the case of OptimalFilterChargeX we want some RQ's created for both QI/QO channels, and some 
      //RQ's shared by the pair.  Each RQs passed in by this class will be flagged via the initial value 
      //to specify which RQ's are only written out for the pair.  We check it here and pass the 
      //correct flag to AddZipListVar.
      if(rqListItr->second == -123456.)
 	 checkMultID = "chargeshared"; //flag to save only one RQ w/ prefix "QS"


      if(rqListItr->second == -987654.)
 	 checkMultID = "chargechanshared"; //flag to save all channels + one RQ w/ prefix "QS"


      //similarly for phonons, if we ever need it
      if(rqListItr->second == -1234567.)
 	 checkMultID = "phononshared"; //flag to save only one RQ w/ prefix "PS"

      AddZipListVar(rqListItr->first, zipNum, checkMultID); 
   }

   return;

}

// ==================== Setting Values  ==========================

void BatOutputManager::SetEventListVal(const string& varName, double val)
{
   //check that var exists
   if(fEventListMap.count(varName) == 0)
   {
      cout <<"BatOutputManager::ERROR! Trying to store event value with invalid name: " << varName 
	   <<".\nCheck ConstructEventOutputList and StoreEventOutput()!" << endl;
      exit(1);
   }

   fEventListMap[varName] = val;

   //cout <<"Storing var: " << varName <<"= " << fEventListMap[varName] << endl;

   return;
}

void BatOutputManager::SetEventListVal(const map<string,double>& rqList)
{
  map<string,double>::const_iterator rqListItr = rqList.begin();
  for( ; rqListItr!=rqList.end(); rqListItr++)
  {
    SetEventListVal(rqListItr->first, rqListItr->second); 
  }

  return;
}

void BatOutputManager::SetVetoListVal(const string& varName, double val, int panelNum)
{

   //attach the panel number
   string panelID = Form("%d",panelNum);
   if(panelID.length() < 2) panelID = "0" + panelID; //all veto prefixes have 2 digits
   string fullRQName = varName + panelID;
   
   if(fVetoListMap.count(fullRQName) == 0)
   {
      cout <<"BatOutputManager::ERROR! Trying to store veto value with invalid name: " << fullRQName 
	   <<".\nCheck ConstructVetoOutputList and StoreVetoOutput()!" << endl;
      exit(1);
   }

   fVetoListMap[fullRQName] = val;
   
   if(fDebugOn) cout <<"Storing for veto RQ " << fullRQName <<", val = " << val << endl;

   return;
}

//for setting values in a whole list of RQs
void BatOutputManager::SetVetoListVal(const map<string,double>& rqList, int panelNum)
{
    //Loop over list and call SetVetoListVal for single RQs
    map<string,double>::const_iterator rqListItr = rqList.begin();
    for( ; rqListItr!=rqList.end(); rqListItr++)
    {
       SetVetoListVal(rqListItr->first, (double)rqListItr->second, panelNum); 
    }
   
   return;
}


void BatOutputManager::SetNoiseMonitorListVal(const string& varName, double val, const string& chName)
{

   //attach the det number
   string fullRQName = chName + varName;
   
   if(fNoiseMonitorListMap.count(fullRQName) == 0)
   {
      cout <<"BatOutputManager::ERROR! Trying to store noisemon value with invalid name: " << fullRQName 
	   <<".\nCheck ConstructNoiseMonitorOutputList and StoreNoiseMonitorOutput()!" << endl;
      exit(1);
   }

   fNoiseMonitorListMap[fullRQName] = val;
   
   if(fDebugOn) cout <<"Storing for noisemon RQ " << fullRQName <<", val = " << val << endl;

   return;
}

//for setting values in a whole list of RQs
void BatOutputManager::SetNoiseMonitorListVal(const map<string,double>& rqList, const string& chName)
{
    //Loop over list and call SetNoiseMonitorListVal for single RQs
    map<string,double>::const_iterator rqListItr = rqList.begin();
    for( ; rqListItr!=rqList.end(); rqListItr++)
    {
       SetNoiseMonitorListVal(rqListItr->first, (double)rqListItr->second, chName); 
    }
   
   return;
}


//Setting the value so that it can be stored in the output
void BatOutputManager::SetZipListVal(const string& varName, const string& chanName, double val, int detNum)
{
   //attach the channel prefix
   string fullRQName = chanName + varName;
   
   //variable name is stored internally with detNum prefix
   string tempString = Form("%d_%s", detNum, fullRQName.c_str()); 
      
  
   //retrive the map that stores the RQs
   map< int, map<string,double> >::iterator zipMap = fMapOfZipMaps.find(detNum);
   if(zipMap == fMapOfZipMaps.end())
   {
     cout <<"BatOutputManager::SetZipListVar ERROR!  Cannot find a map for specified detector in fMapOfZipMaps."
	  << endl;
     exit(1);
   }  

   //check against master list, if not found...
   if((zipMap->second).count(tempString) == 0)
   {
      bool isFound = false;

      //Check if its stored as "QS" for charge channels
      if(chanName == "QI" || chanName == "QO" ||
         chanName == "QIS1" || chanName == "QOS1" ||
         chanName == "QIS2" || chanName == "QOS2")
      {  
         string testRQName = "QS" + varName;

         if (chanName.length()==4)
	    testRQName = "QS" + chanName.substr(3) + varName;

	 string testString = Form("%d_%s", detNum, testRQName.c_str()); 
     
	 if((zipMap->second).count(testString) == 1)
	 {
	    tempString = testString; //overwrite with correct name
	    isFound = true;
	 }
      } //endif charge channel

      //...or stored as "PS" for phonon channels - FIXME fix for izips
      if(chanName == "PA" || chanName == "PB" || chanName == "PC" || chanName == "PD") 
      {
	 string testRQName = "PS" + varName;
	 string testString = Form("%d_%s", detNum, testRQName.c_str()); 
	 if((zipMap->second).count(testString) == 1)
	 {
	    tempString = testString; //overwrite with correct name
	    isFound = true;
	 }	 

      } //endif phonon channel

      if(!isFound)
      {
	cout <<"BatOutputManager::SetZipListVar ERROR! Unable to find valid rq value with name: " << varName 
	     <<" for chan type " << chanName <<" on detector " << detNum 
	     <<".\nCheck Constructruction of RQ list!" << endl;
	 exit(1);
      }

   } //end search against master list
   
  
   // Check if both single pulse and shared RQs are available
   // Set one of the variable to -99999 if not used based 
   // on broken side list
   

   if(chanName == "QI" || chanName == "QO" ||
      chanName == "QIS1" || chanName == "QOS1" ||
      chanName == "QIS2" || chanName == "QOS2")
    {
        
        // check if shared and single RQs available
        string side = "S";
        if (chanName.find("S1") != string::npos)  side = "S1";
        if (chanName.find("S2") != string::npos)  side = "S2";

        string sharedRQName = "Q" + side + varName;
        string sharedString = Form("%d_%s", detNum, sharedRQName.c_str()); 

        if(((zipMap->second).count(sharedString) == 1) && 
           ((zipMap->second).count(tempString) == 1)) 
         {

             // get broken side list
             vector<string> brokenSides;    
             if (fUserData.DoRead("DET_STATUS_FILE"))
               brokenSides = fUserData.GetBrokenChargeSideList(detNum);  
         
             // check if channel "broken" 
             if (find(brokenSides.begin(),brokenSides.end(), side) != brokenSides.end()) 
              { 
               // case broken side
               // => Qshared = -99999. and  QI or QO = val 

               (zipMap->second)[tempString] = val;  
               (zipMap->second)[sharedString] =  -999999.;
             
              } else {
               // case not broken
               (zipMap->second)[tempString] = -999999.;  // store 
               (zipMap->second)[sharedString] = val;  // store 
              }
         
            // that's it, values stored
            return;
         }
     }
            
   


   //now store the value!
   (zipMap->second)[tempString] = val;
  
   if(fDebugOn) cout <<"Storing variable: " << tempString <<"= " << (zipMap->second)[tempString] <<", for det = " << detNum << endl;
  
  return;
}

//for setting values in a whole list of RQs
void BatOutputManager::SetZipListVal(const map<string,double>& rqList, const string& chanName, int detNum)
{
   //Loop over list and call SetZipListVal for single RQs
   map<string,double>::const_iterator rqListItr = rqList.begin();
   for( ; rqListItr!=rqList.end(); rqListItr++)
   {
      SetZipListVal(rqListItr->first, chanName, (double)rqListItr->second, detNum); 
   }
   
   return;
}

// ==================== Utility Functions  ==========================

//this function only works for zips at the moment, but its simple to add the option for blips
vector<string> BatOutputManager::GetChanList(const string& multID, int detType)
{
   vector<string> chanList;
   //check that a valid flag was passed
   if(multID != "allchan" && multID != "phononchan" && multID != "chargechan" && multID != ""
      && multID != "chargeshared" && multID != "chargechanshared" && multID != "phononshared" && multID != "chargechantotal" 
      && multID != "phononchantotal" && multID != "phonontotal" && multID != "phononsides"  )
   {     
      cout <<"BatOutputManager::GetChanList ERROR!  Invalid flag for initializing zip variable list, check ConstructOutputLists! " <<multID << endl;
      exit(1);
   }

   if(multID == "")
      chanList.push_back("");
  
   //all channels
   if(multID == "allchan")
   {
      ChannelMapHelper::FillAllChannelList(detType, chanList);
   }
   
   //all phonon channels
   if(multID == "phononchan")
   {
      ChannelMapHelper::FillPhononChannelList(detType, chanList);
   }


   //all phonon channels, plus PT
   if(multID == "phononchantotal")
   {
      ChannelMapHelper::FillPhononChannelList(detType, chanList);
      chanList.push_back("PT");
   }
 
   // PT only
   if(multID == "phonontotal") {
     chanList.push_back("PT");
   }

    // sum of  phonon channels per side
   if(multID == "phononsides")
   {
      chanList.push_back("PS1");
      chanList.push_back("PS2");
   }

   
   //all charge channels
   if(multID == "chargechan")
   {
      ChannelMapHelper::FillChargeChannelList(detType, chanList);
   }

   //all charge channels, plus QT
   if(multID == "chargechantotal")
   {
      ChannelMapHelper::FillChargeChannelList(detType, chanList);
      chanList.push_back("QT");
   }

   //special RQ shared by all charge channels
   if(multID == "chargeshared")
   {
      if (detType==BatRootTypes::kiZIPSoudan) {
       chanList.push_back("QS1");
       chanList.push_back("QS2");
      } else {
       chanList.push_back("QS");
      }
    }

    

   //all charge channels + special RQ shared by all charge channels
   if(multID == "chargechanshared")
   {
      ChannelMapHelper::FillChargeChannelList(detType, chanList);
      if (detType==BatRootTypes::kiZIPSoudan) {
       chanList.push_back("QS1");
       chanList.push_back("QS2");
      } else {
       chanList.push_back("QS");
      }
    }




   //special RQ shared by all phonon channels
   if(multID == "phononshared")
   {
      chanList.push_back("PS");
   }

 
   //a redundant check
   if(chanList.size() == 0) 
   { 
      cout <<"BatOutputManager::GetChanList WARNING: Channel list is empty" << endl;
      //exit(1);
   }

   return chanList;
}

//Makes a dummy instance of the analysis class in order to retrieve the RQ list
//a little clumsy at the moment, but I'm not able to think of a better way [LLH]
map<string,double> BatOutputManager::GetRQList(const string& analysisName, const string& sensorType)
{

    map<string,double> rqList;
    bool found = false;

    if(analysisName == "PipeFitPhonon")
    {
       PipeFitPhonon emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "OptimalFilterPhonon")
    {
       OptimalFilterPhonon emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "OptimalFilterPhononDMC")
    {
       OptimalFilterPhonon emptyAnalysis("OptimalFilterPhononDMC");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "OptimalFilterPhononGlitch1")
    {
       OptimalFilterPhonon emptyAnalysis("OptimalFilterPhononGlitch1");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "OptimalFilterPhononLFnoise1")
    {
       OptimalFilterPhonon emptyAnalysis("OptimalFilterPhononLFnoise1");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }
   

    if(analysisName == "OptimalFilterPhononPileup")
    {
       OptimalFilterPhonon emptyAnalysis("OptimalFilterPhononPileup");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "TailFitPhonon")
    {
       SingleExponentialFit emptyAnalysis("TailFitPhonon");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "OptimalFilterChargeX")
    {
       OptimalFilterChargeX emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    if(analysisName == "OptimalFilterCharge")
    {
       OptimalFilterNxN emptyAnalysis("OptimalFilterCharge");
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }

    //additional analysis options go here - DO NOT modify or copy this comment (for auto_analysis)

   if(analysisName == "SimulateFromRandoms")
   {
      SimulateFromRandoms emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }

   if(analysisName == "OptimalFilterPhonon1X2")
     {
       OptimalFilterPhonon1X2 emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
     }
   
   if(analysisName == "OptimalFilterPhononNS")
   {
      OptimalFilterPhononNS emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }


   if(analysisName == "OptimalFilterCharge2X2")
   {
      OptimalFilterNxN emptyAnalysis("OptimalFilterCharge2X2");
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }


   if(analysisName == "PSDIntegralPhonon")
   {
      PSDIntegralPhonon emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }


   if(analysisName == "WedgeFitPhonon")
   {
      WedgeFitPhonon emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }


   if(analysisName == "ConstFreqRTFTWalkPhonon")
   {
      ConstFreqRTFTWalkPhonon emptyAnalysis;
      rqList = emptyAnalysis.GetRQList();
      found = true;
   }


    if(analysisName == "F5ChargeX")
    {
       F5ChargeX emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "InflectionTime")
    {
       InflectionTime emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "NoiseSelector")
    {
       NoiseSelector emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "PulseIntegral")
    {
       PulseIntegral emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "VarFreqRTFTWalkPhonon")
    {
       VarFreqRTFTWalkPhonon emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "RTFTWalkCharge")
    {
       RTFTWalkCharge emptyAnalysis;
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(analysisName == "BasicPulseCalc")
    {
       BasicPulseCalc emptyAnalysis(sensorType);
       rqList = emptyAnalysis.GetRQList();
       found = true;
    }


    if(!found)
    {
       cout <<"BatOutputManager::GetRQList ERROR!" 
 	   <<" Requested RQList for: " << analysisName <<", does not exist! Check that this class is properly registered in BatOutputManager or for spelling errors in the config file." << endl;
       exit(1);
    }
      
    return rqList;
}


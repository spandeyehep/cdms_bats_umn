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

#ifndef BATOUTPUTMANAGER_H
#define BATOUTPUTMANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <list>

#include "TFile.h"

#include "EventBuilder.h"
#include "DetectorConfigManager.h"

using namespace std;

//!This class defines, manages and stores the RQ's.  The output is generated as ROOT TTree objects (see header file for more information).
class BatOutputManager 
{
   public:
 
      // constructor
      BatOutputManager(UserDataManager& myUserData, DetectorConfigManager& myDetectorConfigManager,
		       const string& rawDataFilename); 

      // destructor
      ~BatOutputManager();      

      // For constructing the event/ZIP output lists
      void ConstructOutputLists();

      //Copy data from EventBuilder and store
      void StoreOutput(const EventBuilder& eventBuilder); //do I need a copy constructor for eventBuilder?

      //Event and ZIP Trees management
      void ConfigureOutputTrees(); //can be private
      void FillTrees();
      void WriteTrees();

      //User Settings Tree management
      void CreateAndWriteSettingsTree();

      //Detector Configuration Tree management
      void CreateAndWriteDetectorConfigTree();
 
      //Filter Tree management
      void CreateAndWriteFilterTree(FilterDataManager& myFilterData);
      
      //Processing Info Tree  management
      void CreateAndWriteProcessingInfoTree(string date, string gitTag_cdmsbats,string gitTag_batcommon);

    private:
      
      //OutputFile
      TFile* fOutputFile;

      //ExtDataMan
      UserDataManager fUserData;
      
      //RQ maps
      map<string,double> fEventListMap;
      map<string,double> fVetoListMap;
      map<string,double> fNoiseMonitorListMap;
      map< int, map<string,double> > fMapOfZipMaps; //map of zip RQ maps

      // list of detectors
      map< int,int > fDetectorMap;
   
      //Output RQ Trees
      TTree* fEventTree;
      TTree* fVetoTree;
      TTree* fNoiseMonitorTree;
      vector<TTree*> fZipTreeVector; 
      
      //To prevent user from adding entries to the list
      //after the branches are set
      bool fEventListLocked;
      bool fZipListLocked;
      bool fVetoListLocked;
      bool fNoiseMonitorListLocked;

      //for debugging
      bool fDebugOn;
   
      //For constructing output lists
      map<string,double> GetRQList(const string& analysisName, const string& sensorType);

      void ConstructEventOutputList();
      void ConstructZipOutputList();
      void ConstructVetoOutputList();
      void ConstructNoiseMonitorOutputList();

      void AddEventListVar(const string& varName); //add one rq per varName
      void AddEventListVar(const map<string,double>& rqList);

      void AddVetoListVar(const string& varName); //add one rq per veto panel per varName
      void AddVetoListVar(const map<string,double>& rqList);

      void AddNoiseMonitorListVar(const string& varName); //add one rq per channel per name
      void AddNoiseMonitorListVar(const map<string,double>& rqList);

      //adds RQs for specified zip
      void AddZipListVar(const string& varName, const int zipNum, const string& multID=""); //add one rq per zip channel per varName 
      void AddZipListVar(const map<string,double>& rqList, const int zipNum, const string& multID="");

      //For storing data in the lists
      void StoreEventOutput(const EventBuilder& eventBuilder); 
      void StoreZipOutput(const EventBuilder& eventBuilder); 
      void StoreVetoOutput(const EventBuilder& eventBuilder); 
      void StoreNoiseMonitorOutput(const EventBuilder& eventBuilder); 

      
      void SetEventListVal(const string& varName, double val);
      void SetEventListVal(const map<string,double>& rqList);
     
      void SetVetoListVal(const string& varName, double val, int panelNum);
      void SetVetoListVal(const map<string,double>& rqList, int panelNum);
      
      void SetNoiseMonitorListVal(const string& varName, double val, const string& chName);
      void SetNoiseMonitorListVal(const map<string,double>& rqList, const string& chName);
      
      void SetZipListVal(const string& varName, const string& chanName, double val, int detNum);
      void SetZipListVal(const map<string,double>& rqList, const string& chanName, int detNum); //for setting a whole list of RQs

      //utility functions
      vector<string> GetChanList(const string& multID, int detType);
      void ResetLists();


      //other
      DetectorConfigManager fDetectorConfigManager;  

};

#endif /* BATOUTPUTMANAGER_H */

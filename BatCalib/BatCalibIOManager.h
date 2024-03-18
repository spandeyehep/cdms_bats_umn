///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: BatCalibIOManager
//Authors: L. Hsu, B. Serfass
//Description:  This class controls the generation of the output files and the reading of the input files.   It defines and manages 
//a series of maps which contain the input RQ's and their values.  This class borrows heavily from the output file generating class in BatRoot (BatOutputManager).
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BATOUTPUTMANAGER_H
#define BATOUTPUTMANAGER_H

#include <iostream>
#include <map>

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"

#include "UserDataManager.h"

using namespace std;

//!Add comemnts here
class BatCalibIOManager 
{
   public:
 
      // constructor
      BatCalibIOManager(const UserDataManager& myUserData, const string& inputSeries, const string& dumpName); 

      // destructor
      ~BatCalibIOManager();      

      // --- File Reading functions  ---

      int  LoadTree(const string& dir, const string& treename);
      void DeleteActiveTree();
      void AddFriendTree(const string& dir, const string& treename);
      int  Activate(const string& varName); //include option for phonon, charge, veto or all?

      bool DoesRQFilePredate(const string& date);

      void ReadNextEntry(int eventCtr);
      int  GetMaxEntries();  //gets maximum entries for active tree
      string  GetSeriesString(){ return fInputSeries; } //get the series string for implementing SeriesStartTime [ANV]
      string  GetSeriesStringWithoutUnderscore(); //get the series string without underscore [ANV]
      double Get(const string& varName);

      void FillOFResolution(int detNum, vector<double>& delaySig, vector<double>& ampSig); //Temporary for CDMS2
      void FillOFResolution(int detNum, int detType, vector<double>& delaySig, vector<double>& ampSig);
      bool IsOFresFilled() {return fIsOFresFilled;};

      map<int, int> GetDetectorMap(); //key = detector number, val = detector code 
      void ReadBatRootFlags();
      int  CheckBatRootChargeAlg(const string& algName, int detNum);
      int  CheckBatRootPhononAlg(const string& algName, int detNum);
      bool  CheckBatRootUserSettingsFlags(const string& flagName);

      // --- File Writing functions  ---

      void CreateAndWriteFilterTree();
      void CreateAndWriteProcessingInfoTree(string date, string gitTag_cdmsbats,string gitTag_batcommon);

      void ConstructOutputRRQTree(map<string, double>* rrqList, const string& treeName);
      void FillOutputRRQTree();
      void WriteOutputRRQTree();


    private:
      
      //misc
      bool   fDebugOn;
      string fInputSeries;
      string fInputDataPathName;
      string fOutputDataPathName;
      bool fIsOFresFilled;  


      //File handles
      TFile* fFileWriter; 

      //ExtDataMan 
      UserDataManager fOptions; 

      //File Reading variables - FIXME the map below probably doesn't need to be a pointer to double
      TChain*               fActiveReadTree;
      map<string, double*>  fActiveBranchMap;  //only select branches within tree are active for reading

      //The key = algorithm name, val = vector of 1's and 0's stating whether routine was on or off
      //val vector index = detNum - 1
      map<string, vector<int> > fBatRootChargeAlgMap; //flags stating the status of the BatRoot charge algorithms
      map<string, vector<int> > fBatRootPhononAlgMap; //flags stating the status of the BatRoot phonon algorithms
      map<string, int > fBatRootUserSettingsFlagMap; //flags from BatRoot user settings 

      //File Writing variables
      TTree*                fOutputRRQTree;
      

};




#endif /* BATOUTPUTMANAGER_H */

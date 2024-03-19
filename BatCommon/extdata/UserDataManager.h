////////////////////////////////////////////////////////////////////////////////////
//Class Name: UserDataManager
//Authors: B. Serfass
//Description:  This class reads the user settings files, store the informations,
//and manage access to to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Feb. 2011 - Renamed this class from UserDataManager to UserDataManager, to avoid
//            confusion withthe new classes DetectorConfigData and
//            DetectorConfigManager [LLH].
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef USERDATAMANAGER_H
#define USERDATAMANAGER_H

#include "ListManager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <vector>

using namespace std;

class UserDataManager {
public:
  // ---- constructor/destructor -----
  UserDataManager();
  ~UserDataManager();      
     
  //  ----- read file ---------
  void ReadFile(string filename, bool overwriteFlag = false);
  void ProcessLine(string line, bool overwriteFlag = false, string filename="", int lineNumber=0);
  
  // ----- get functions -------

  // path
  string GetPath(const string& varName) const {return ListManager::GetParameter(fMapString,"PATH_"+varName); };
  string GetPrefix(const string& varName) const {return ListManager::GetParameter(fMapString,"PREFIX_"+varName); };

  // read external files
  bool DoRead(const string& varName, bool required=false) const;     
  
  // general settings
  int GetMaxEvents() const { vector<int> intVect = ListManager::GetParameter(fMapVectInt,"MAX_EVENTS");  return intVect[0];};
  int GetMaxZIPs() const { vector<int> intVect = ListManager::GetParameter(fMapVectInt,"MAX_ZIPS");  return intVect[0];};

  // processing flags 
  bool DoEventTimeProcessing() const {vector<int> intVect =  ListManager::GetParameter(fMapVectInt,"DO_PROCESSING_EVENTTIME");  return (bool) intVect[0];} 
  bool DoTriggerProcessing() const {vector<int> intVect =  ListManager::GetParameter(fMapVectInt,"DO_PROCESSING_TRIGGER");  return (bool) intVect[0];} 
  bool WriteTriggerRQ()    const {vector<int> intVect =  ListManager::GetParameter(fMapVectInt,"WRITE_RQ_TRIGGER"); return (bool) intVect[0]; }  
  
  bool DoVetoProcessing() const {vector<int> intVect =  ListManager::GetParameter(fMapVectInt,"DO_PROCESSING_VETO");  return (bool) intVect[0];} 
  bool WriteVetoRQ()    const {vector<int> intVect =  ListManager::GetParameter(fMapVectInt,"WRITE_RQ_VETO"); return (bool) intVect[0]; }  
  
  ///Do noise monitor processing for a given detector (or ANY detector if chName is blank)?
  bool DoNoiseMonitorProcessing(const string& chName="") const;
  bool WriteNoiseMonitorRQ(const string& chName="") const;
  
  
  bool DoZipProcessing(int detNum) const; 
  bool WriteZipRQ(int detNum)    const; 
  

  // Allow modification of Raw Data
  bool DoModifyRawData() const;
  map<int,string> GetRawDataModificationMap() const;


  // template 
  bool CalcChargeTemplate(int detNum) const;
  bool CalcPhononTemplate(int detNum) const;
  
  // ZIP configuration
  string GetTowerName(int detNum) const { return  ListManager::GetParameter(fZipMapOfMapString.find(detNum)->second,"DET_TOWER"); };  
  int GetTowerNumber(int detNum)  const; 

  bool IsGe(int detNum) const;
  bool IsSi(int detNum) const;
  
  // risetime,falltime for template shape
  double GetRiseTime(int detNum,int detType, const string& channel) const;
  double GetFallTime(int detNum,int detType, const string& channel) const;
  
  // tail fit fall time
  double GetTailFitFallTime(int detNum,int detType, const string& channel) const;
  double GetTailFitFallTimeSigma(int detNum,int detType, const string& channel) const;

  // Analysis algorithm: sensorType = "phonon" or "charge"
  bool DoAlgorithm(int detNum, const string& sensorType,const string& name) const;
  bool DoZipAlgorithm(int detNum, const string& name) const;

  list<string> GetListAlgorithms(int detNum, const string& sensorType) const;
  list<string> GetListAlgorithms(const string& detName, const string& sensorType) const;
  
  // relative phonon calibration  for sum of pulses
  double GetRelativeCalibration(int detNum, int detType, const string& channel) const;

  // Overall phonon calibration (might be multiple parameters in case of dependence with temperature)
  // varName = "psumOF" or "ptNF" or "ptOF"
  vector<double> GetOverallCalibration(int detNum, const string& varName, vector<string> brokenChannels = vector<string>()) const;  
 

  // get calibrations from cdmsbats config file
  double GetOverallPTCalibrationTI(int detNum, int detType) const;
  double GetQCalibration(int detNum, int detType, const string& channel) const;



  //  flag to use channel for event selection (through PT, QT), return true by default
  bool UseChannelNoiseSelection(int detNum, int detType, const string& channel) const;

  // Detector status, channel can be also LEDs
  int GetDetectorStatus(int detNum, const string& channel, const string&  inputSeries);

  // ALL OTHER PARAMATERS: Get double/int/string parameters
  string GetStringParameter(const string& varName) const 
  { vector<string> stringVect = ListManager::GetParameter(fMapVectString,varName); return stringVect[0]; } 
  string GetStringParameter(int detNum, const string& varName) const
  { vector<string> stringVect =  ListManager::GetParameter(fZipMapOfMapVectString.find(detNum)->second,varName); return stringVect[0];}
  
  vector<string> GetVectStringParameter(const string& varName) const    
  { return ListManager::GetParameter(fMapVectString,varName);} 
  vector<string> GetVectStringParameter(int detNum, const string& varName)  const
  { return ListManager::GetParameter(fZipMapOfMapVectString.find(detNum)->second,varName);}
  
  int GetIntParameter(const string& varName) const 
  { vector<int> intVect = ListManager::GetParameter(fMapVectInt,varName);  return intVect[0] ;} 
  int GetIntParameter(int detNum, const string& varName) const
  { vector<int> intVect =  ListManager::GetParameter(fZipMapOfMapVectInt.find(detNum)->second,varName);  return intVect[0];}

  vector<int> GetVectIntParameter(const string& varName) const   
  { return ListManager::GetParameter(fMapVectInt,varName);} 
  vector<int> GetVectIntParameter(int detNum, const string& varName) const   
  { return ListManager::GetParameter(fZipMapOfMapVectInt.find(detNum)->second,varName);}

  double GetDoubleParameter(const string& varName) const 
  { vector<double> doubleVect = ListManager::GetParameter(fMapVectDouble,varName); return doubleVect[0]; } 
  double GetDoubleParameter(int detNum, const string& varName) const
  { vector<double> doubleVect =  ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,varName); return doubleVect[0];}

  vector<double> GetVectDoubleParameter(const string& varName) const   
  { return ListManager::GetParameter(fMapVectDouble,varName);} 
  vector<double> GetVectDoubleParameter(int detNum, const string& varName) const
  { return ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,varName);}
  

  // check if parameter exist 
  bool HasDoubleParameter(const string& varName){
    return ListManager::HasParameter(fMapVectDouble,varName);}
 
  bool HasIntParameter(const string& varName){
    return ListManager::HasParameter(fMapVectInt,varName);}
  
  bool HasStringParameter(const string& varName){
    return ListManager::HasParameter(fMapVectString,varName);}

  bool HasDoubleParameter(int detNum,const string& varName);
  bool HasIntParameter(int detNum,const string& varName);
  bool HasStringParameter(int detNum,const string& varName);
  
   

  // Status RQ list 
  void DoCalcDetectorStatus(const string &inputSeries);
  void ConstructZipRQList(const map<int,int>& detectorMap);
  map<int, map<string, double> > GetAllZipRQList() const { return fZipRQList; }
  map<string, double> GetZipRQList(const int&  detNum) const { return (fZipRQList.find(detNum))->second; }

  // Broken channel list
  void FillBrokenChannelLists();
  vector<string> GetBrokenChannelList(const int&  detNum) const { return (fBrokenChannelMap.find(detNum))->second;}
  vector<string> GetBrokenPhononChannelList(const int&  detNum) const { return (fBrokenPhononChannelMap.find(detNum))->second;}
  vector<string> GetBrokenChargeChannelList(const int&  detNum) const { return (fBrokenChargeChannelMap.find(detNum))->second;}
  vector<string> GetBrokenChargeSideList(const int&  detNum) const { return (fBrokenChargeSideMap.find(detNum))->second;}





  // config RQs
  map<string, vector<double> > GetVectDoubleRQList() { return fVectDoubleRQList; }
  map<string, vector<int> > GetVectIntRQList() const { return fVectIntRQList; }
  vector<string>GetFileNameList() const { return fFileNameList; }
    
  //allow programmatic setting of these values
 public:
  // ---- store ----
  void StoreParameter(string key,string keyType, list<int> detectorList, vector<string> valueStrVect, int lineNumber,
		      bool overwriteFlag = false);

  // ---- RQ list ---
  void ConstructConfigRQList();

  // ---- set functions  ----  
  void SetStringParameter(const string& varName, string val, bool overwriteFlag) 
  { SetTypeParameter(fMapString,varName,val, overwriteFlag); };
  
  void SetStringParameter(int detNum, const string& varName, string val, bool overwriteFlag)
  { SetTypeParameter(fZipMapOfMapString,detNum, varName,val,overwriteFlag); };  
  
  void SetVectStringParameter(const string& varName, vector<string> valVect, bool overwriteFlag) 
  { SetTypeParameter(fMapVectString,varName,valVect, overwriteFlag); };
           
  void SetVectStringParameter(int detNum, const string& varName, vector<string> valVect, bool overwriteFlag)
  { SetTypeParameter(fZipMapOfMapVectString,detNum, varName, valVect, overwriteFlag); };  

  void SetVectIntParameter(const string& varName, vector<int> valVect, bool overwriteFlag)
  { SetTypeParameter(fMapVectInt,varName,valVect,overwriteFlag) ;};
   
  void SetVectIntParameter(int detNum, const string& varName, vector<int> valVect, bool overwriteFlag)
  { SetTypeParameter(fZipMapOfMapVectInt,detNum, varName,valVect,overwriteFlag) ;};

  void SetVectDoubleParameter(const string& varName, vector<double> valVect, bool overwriteFlag)
  { SetTypeParameter(fMapVectDouble,varName,valVect,overwriteFlag) ;};
        
  void SetVectDoubleParameter(int detNum, const string& varName, vector<double> valVect, bool overwriteFlag)
  { SetTypeParameter(fZipMapOfMapVectDouble,detNum, varName,valVect,overwriteFlag) ;};
   
  template<class Type> void SetTypeParameter(map<string,Type> &aMapT, const string& varName, Type val, bool overwriteFlag);
  template<class Type> void SetTypeParameter(map<int, map<string,Type> > &aMapOfMapT, int detNum, const string& varName, Type val, bool overwriteFlag);

  // RQs list
 private:
  // list of detectors
  list<int> ReadDetectorList(string detector,int lineNumber);
  string trim(string str);
  vector<string> Tokenize(string aStr);
  
  // data container
  map<string,string>  fMapString;
  map<string,vector<string> >  fMapVectString;   
  map<string,vector<int> >  fMapVectInt; 
  map<string,vector<double> >  fMapVectDouble;   
  
  map< int, map<string,string> > fZipMapOfMapString;
  map< int, map<string,vector<string> > > fZipMapOfMapVectString;
  map< int, map<string,vector<int> > >  fZipMapOfMapVectInt;
  map< int, map<string,vector<double> > >  fZipMapOfMapVectDouble; 
  
  // data container for dumping config RQs
  map<string,vector<double> > fVectDoubleRQList;
  map<string,vector<int> > fVectIntRQList;
  vector<string>  fFileNameList;
  
  // data container for ZIP RQs
  map<int, map<string, double> > fZipRQList;

  // data container to store broken channels
  map<int, vector<string> > fBrokenChannelMap;
  map<int, vector<string> > fBrokenPhononChannelMap;
  map<int, vector<string> > fBrokenChargeChannelMap; 
  map<int, vector<string> > fBrokenChargeSideMap; 



};

#endif /* USERDATAMANAGER_H */

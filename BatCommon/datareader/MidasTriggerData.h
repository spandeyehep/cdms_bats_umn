///////////////////////////////////////////////////////////////////////
//
//  Class Name:         MidasTriggerData
// 
//  Author (s):         B. Serfass, modified version of TTriggerData
//                                  (Thomas Lindner - TRIUMF)
//
//  Description:       C++ class representing one trigger. Data members are filled 
//                     by MidasEventData (pulses stores in vector<PulseData>
//
//
///////////////////////////////////////////////////////////////////////


#include <vector>
#include "PulseData.h"


class MidasTriggerData {

  friend class MidasEventData;

public:

  
  MidasTriggerData(){};
  MidasTriggerData(int triggerSourceTower,int triggerSourceDCRC,int triggerWord,int triggerNumber):
  fTriggerSourceTower(triggerSourceTower),
    fTriggerSourceDCRC(triggerSourceDCRC),
    fTriggerWord(triggerWord),
    fTriggerNumber(triggerNumber){
  
};
  
  //destructor
  ~MidasTriggerData(){
  }
  
  //  MidasTriggerData(int triggerID,int triggerType,int triggerNumber):
  //copy constructor
  MidasTriggerData(MidasTriggerData &other){
    fTriggerSourceTower = other.fTriggerSourceTower;
    fTriggerSourceDCRC = other.fTriggerSourceDCRC;
    fTriggerWord = other.fTriggerWord;
    fTriggerNumber = other.fTriggerNumber;
    for(size_t i = 0; i<other.fPulseDataList.size(); i++){
      fPulseDataList.push_back(other.fPulseDataList[i]);
    }
    cout << "making copy of MidasTriggerData" << endl;
    }

  MidasTriggerData& operator=(const MidasTriggerData &other)
    {
      cout << "assignment of  of MidasTriggerData is not implemented" << endl;
      return *this;
    }

  void ClearPulseDataList(){
    fPulseDataList.clear();
  }

  const int GetTriggerSourceTower() {return fTriggerSourceTower;};
  const int GetTriggerSourceDCRC(){ return fTriggerSourceDCRC;};
  const int GetTriggerWord(){ return fTriggerWord;};
  const int GetTriggerNumber(){ return fTriggerNumber;};
  const int GetEventCategory(){ return fEventCategory;};
  
  void SetEventCategory(int category){ fEventCategory=category;};

private:
  // trigger source: tower
  int fTriggerSourceTower;

  // trigger source: DCRC
  int fTriggerSourceDCRC;

  // trigger source: DCRC
  int fTriggerWord;

  // trigger Number
  int fTriggerNumber;

  /// map with Pulse data
  vector<PulseData>  fPulseDataList; 

  //some parameters
  uint32_t fEventCategory;
};
  

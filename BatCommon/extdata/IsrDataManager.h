///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: IsrDataManager
//Authors: B. Serfass
//Description:  This class read the ISR file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef ISRDATAMANAGER_H
#define ISRDATAMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "ListManager.h"

using namespace std;



class IsrDataManager
{


   public:

    // ==== constructor/destructor ====
    IsrDataManager(const int& maxZIPs);
    IsrDataManager();

    ~IsrDataManager();      
  
    // ==== Read file ====
    void ReadFile(const string& filename);
     
 
    // ==== Calc ISR RQs for all channels ====
    void DoCalcLastIsrTime(const double& eventTime);
    void DoCalcBias(const int& detNum,const double& eventTime);    
   

    // ==== Get infos ====
   
    // channel ID = "PA", "PB","PC","PD", "QI", "QO"
    double GetDriverGain(const int& detNum, const string& channel,  const double& eventTime) const;
    double GetBias(const int& detNum, const string& channel, const double& eventTime) const;
    double GetBiasTime(const int& detNum, const string& channel, const double& eventTime) const;
    bool IsConfigured(const int& detNum) const;
    


    // channel ID = "QI", "QO"  (return both "Qenabled" and "QenabledTime" inside a map)
    map<string,double>  GetQenabled(const int& detNum, const string& channel, const double& eventTime) const;
      
   
    // Last ISR time any detector/operation
    double GetLastIsrTime(const double& eventTime) const;    


    // ==== RQ list ====

    // Event RQs
    void ConstructEventRQList();
    map<string, double> GetEventRQList() const { return fEventRQList; }
 

    // ZIP RQs
    void ConstructZipRQList(const int& maxZIPs);
    map<int, map<string, double> > GetAllZipRQList() const { return fZipRQList; }
    map<string, double> GetZipRQList(const int& detNum) const { return (fZipRQList.find(detNum))->second; }

    // reset RQ list
    void ResetRQList();
    void ResetZipRQList();
    void ResetEventRQList(); 

   private:


    // ==== set parameter ====      
    void SetDoubleParameter(const int& detNum, const string& varName, double val, bool overwriteFlag);


    // ==== useful functions for reading file ====
    string trim(string str);
    vector<string> Tokenize(string aStr);
   

    // ==== Get data member ====
    map<string,double> GetVal(const int& detNum, const string& keyName, const double& eventTime) const;
    map<string,double> GetVal(map<string,double> &zipMap, const string& keyName, const double& eventTime) const;
    vector<string> GetChanList(const string& multID);


    // ==== data member from ISR file ====
    map< int, map<string,double> > fZipMapOfMapDouble; //ZIPs  map
    vector<double> fISRtime;
    map<string, double> fEventRQList; // RQ list
    map<int, map<string, double> > fZipRQList;
    bool fStoreRQs;
};




#endif /* ISRDATAMANAGER_H */

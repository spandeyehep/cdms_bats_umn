///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: DmmDataManager
//Authors: B. Serfass
//Description:  This class read the DMM file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////




#ifndef DMMDATAMANAGER_H
#define DMMDATAMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <map>

using namespace std;


class DmmDataManager
{


   public:

    // ==== constructor/destructor ====
    DmmDataManager(const map< int, int >& detectorMap);
    DmmDataManager(); //default does nothing
    ~DmmDataManager();      
     

    // ==== read file ====
    void ReadFile(const string& filename);
     
 
    // ==== Calculations/fill RQ list ====
    void DoCalc(const int &detNum, const double& eventTime);
 

    // ==== Get functions ====
    
    // File exists
    bool FileExists() {return fFileExists;};

    // channel ID = "PA", "PB","PC","PD"
    double GetPhononOffset(const int &detNum, const string& channel, const double& eventTime) const;
   
    // Trigger threshold name = "QHi","QLo","PHi","PLo","Whisper"
    double GetTriggerThreshold(const int& detNum, const string& trigName, const double& eventTime) const;
    

    // ==== RQ list  ====
    void ConstructZipRQList();
    map<int, map<string, double> > GetAllZipRQList() const { return fZipRQList; }
    map<string, double> GetZipRQList(const int&  detNum) const { return (fZipRQList.find(detNum))->second; }

    // reset RQ list
    void ResetRQList();
    void ResetZipRQList();


   private:
 
    // ==== read useful functions ====
    string trim(string str);
    vector<string> Tokenize(string aStr);


    // ==== set functions ====
    void SetDoubleParameter(int detNum, const string& keyName, double val, bool overwriteFlag);


    // ==== Get data member ====
    double GetLastDmmTime(const map<string,double> &zipMap, const double& eventTime) const;
    map<string,double> GetVal(const int& detNum, const string& keyName, const double& eventTime) const;
    map<string,double> GetVal(const map<string,double> &zipMap, const string& keyName, const double& eventTime) const;


    // ==== data container ====
    map< int, map<string,double> > fZipMapOfMapDouble; //ZIPs's map
    map<int, map<string, double> > fZipRQList; // RQ list
    bool fStoreRQs;

    // ==== detector list ====
    map< int, int > fDetectorMap;

    // === file exists ===
    bool fFileExists;
  
};




#endif /* DMMDATAMANAGER_H */

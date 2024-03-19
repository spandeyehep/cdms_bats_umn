///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: InfoDataManager
//Authors: B. Serfass
//Description:  This class read the INFO file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef INFODATAMANAGER_H
#define INFODATAMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <string>
#include <algorithm>


#include "ListManager.h"

using namespace std;

class InfoDataManager
{


   public:

    // ---- constructor/destructor -----

     InfoDataManager();
     ~InfoDataManager();      
     
 
   // ---- read file ----
    void ReadFile(string filename);
     
  
    // --- Get infos --- 

    // detNum = 1-30,  sensorType = "phonon" or "charge"
        
   double GetSampleRate(const int& detNum,const string& sensorType) const;
   int GetPreTrigger(const int& detNum,const string& sensorType) const; 
   int GetPostTrigger(const int& detNum,const string& sensorType) const; 
       

   private:

    // set function
    void SetDoubleParameter(const int& detNum, const string& varName, double val, bool overwriteFlag);
   
     // read useful functions
    string trim(string str);
    vector<string> Tokenize(string aStr);

 
   // data container
    map< int, map<string,double> > fZipMapOfMapDouble; //ZIPs map
     

};




#endif /* INFODATAMANAGER_H */

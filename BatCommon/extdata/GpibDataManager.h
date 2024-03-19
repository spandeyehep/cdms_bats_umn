///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: GpibDataManager
//Authors: B. Serfass
//Description:  This class read the GPIB file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


// There are several event tags in the gpib_states_changed.log.  They are:
// 1001 - config (which biases the detectors)
// 1011 - run
// 1021 - pause
// 1031 - resume
// 1000 - stop (which unbiases the detectors)
// 0100 - begin flashing detectors (The flash occurs here)
// 0111 - end flashing detectors (20 min post-flash cooldown completed now)
// 1099 - abort
//
// The event tag can be interpreted as follows:  The right-most digit is 1
// if the detectors are biased, the second right-most digit is a run state
// indicator (0 - config or stop, 1 - run, 2 - pause, 3 - resume, 9 - abort).
// The stop bake should have been 0 for this digit. The next digit is 1 for
// flashing commands and otherwise 0.  The left-most digit flags if this is a
// normal run-state change and is 1 for all commands except for bake and end
// bake.


#ifndef GPIBDATAMANAGER_H
#define GPIBDATAMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <string>
#include <algorithm>

#include "ListManager.h"

using namespace std;

class GpibDataManager
{

   public:

    // ==== constructor/destructor ====
    GpibDataManager();
   ~GpibDataManager();      
     
 
    // ==== read file ====
    void ReadFile(string filename);

     
    // ==== calc Time after flash, flash time ====
    void DoCalc(const double& eventTime);
 

    // ==== Get timing ==== 
    double  GetTimeLastFlash() { return fTimeLastFlash;};
    double  GetTimeAfterFlash() { return fTimeAfterFlash;};
    double  GetTimeBiasOnAfterFlash() {return fTimeBiasOnAfterFlash;};


    // ==== RQ list ====  
    void ConstructRQList();
    void ResetRQList();
    map<string, double> GetRQList() const { return fRQList; }
 
   private:
 
    // ==== set function ====
    void SetParameter(double time, string tag, bool overwriteFlag);
   
    // ==== read useful functions ====
    string trim(string str);
    vector<string> Tokenize(string aStr);

 
    // ==== data container ====
    map<double,string> fMapString;
   
    // flash time
    double fTimeAfterFlash;
    double fTimeBiasOnAfterFlash;
    double fTimeLastFlash;
    double fTimeLastStart;

    // RQ list 
    map<string, double> fRQList;
    bool fStoreRQs;

};




#endif /* GPIBDATAMANAGER_H */

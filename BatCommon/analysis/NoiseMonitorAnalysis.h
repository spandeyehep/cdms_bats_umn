/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: NoiseMonitorAnalysis
//Authors: B. Loer
//Description:  NoiseMonitor pulse analysis algorithm copied from veto analysis
//code.  
//
//File Import By: B. Loer
//Creation Date: Mar 2014
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef NoiseMonitorAnalysis_H
#define NoiseMonitorAnalysis_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!NoiseMonitorAnalysis class.  This class returns quantities derived from the veto pulses.
class NoiseMonitorAnalysis : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      NoiseMonitorAnalysis();  
      ~NoiseMonitorAnalysis(); //destructor 

      //Set parameters
      void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

   private:

      void ConstructRQList();

      //define private functions and data members here
      

};

#endif /* NoiseMonitorAnalysis_H */

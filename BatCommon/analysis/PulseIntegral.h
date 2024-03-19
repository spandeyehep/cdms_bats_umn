/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: PulseIntegral
//Authors: L. Hsu
//Description:  This class returns the integral of the pulse after applying
// a low pass frequency filter. 
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////


#ifndef PulseIntegral_H
#define PulseIntegral_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the PulseIntegral Class.  It serves as a template for all other analysis classes
class PulseIntegral : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      PulseIntegral();  
      ~PulseIntegral(); //destructor 

      //Set parameters
      void SetFilterParameters(const double& sampleRate, const double& cutoff, const int& order) 
      { fSampleRate = sampleRate;
	fFilterCutoff = cutoff;
	fFilterOrder = order;
	return; }

      //do the calculations
      void DoCalc(const vector<double>& aPulse, const string& pulseType);

      //define public functions here
      double GetIntegral() const { return fIntegral; }

   private:

      void ConstructRQList();

      //define private functions and data members here

      //input for filtering
      double fSampleRate;
      double fFilterCutoff;
      int    fFilterOrder;

      //results
      double fIntegral;

};

#endif /* PulseIntegral_H */

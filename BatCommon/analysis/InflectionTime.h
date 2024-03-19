///////////////////////////////////////////////////////////////////////////////// 
//Class Name: InflectionTime
//Author:  L. Hsu
//Description: This class is the equivalent of the PipeFitter
// code that finds the inflection time of a single pulse
//
// Original author of the PipeFitter code: J. Yoo 
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef InflectionTime_H
#define InflectionTime_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the InflectionTime Class.  It computes the maximum and minimum of the pulse derivative.  This algorithm was originally ported from PipeFitter
class InflectionTime : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      InflectionTime();  
      ~InflectionTime(); //destructor 

      //Set parameters
      void SetInflectionWindow(const int& minWindow, const int& maxWindow)
           {  
	      fMinWindow = minWindow;
	      fMaxWindow = maxWindow;
	      return;
	   }

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here

   private:

      void ConstructRQList();

      //constant values go here
      //static const kMyConstant = 42; //follow this example (note constants start with "k")

      //define private functions and data members here
      
      //input parameters
      int fMinWindow; //define window to search for inflection times
      int fMaxWindow; //define window to search for inflection times

      //results of this calculation
      double fMaxDiff;     //maximum differential
      double fMaxDiffTime; //maximum differential time (in adc bins)

      double fMinDiff;     //minimum differential
      double fMinDiffTime; //minimum differential time (in adc bins)

};

#endif /* InflectionTime_H */

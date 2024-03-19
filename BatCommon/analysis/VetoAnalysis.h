/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: VetoAnalysis
//Authors: M. Kos and I. Ruchlin
//Description:  Veto pulse analysis algorithm imported from the PipeFitter  
//code.  
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef VetoAnalysis_H
#define VetoAnalysis_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!VetoAnalysis class.  This class returns quantities derived from the veto pulses.
class VetoAnalysis : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      VetoAnalysis();  
      ~VetoAnalysis(); //destructor 

      //Set parameters
      void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here
      double GetVetoBaseline( const vector<double>& ) const;
      vector<int> GetPeakPos( const vector<double>& ) const;
      double GetPeakAmp(const vector<double>&, int peakPos ) const;

      //set functions
      void SetVetoSampleTime(double sampleTime) {fSampleTime = sampleTime; return;}
      void SetSlopeThresh(double slopeThres) {fSlope = slopeThres; return;}
      void SetVetoTriggerBin(int triggerBin) {fTriggerBin = triggerBin; return;}
      void SetVetoBinToVolts(double binsToVolts) {fBinToVolts = binsToVolts; return;}
      void SetPreTime(double pretime) {fPreTime = pretime; return;} //in microseconds
      void SetBaselineRange(int baselineMin, int baselineMax) {fBaselineMin = baselineMin; fBaselineMax = baselineMax; return;} //range in which to search for baseline
      void SetTriggerOffset(double triggerOffset) {fTriggerOffset = triggerOffset; return;} //in microseconds, time past trigger to look for pulses
      

   private:

      void ConstructRQList();
      double fSlope;
      double fSampleTime;  //in usec
      double fBinToVolts;
      double fPreTime; //in usec
      double fTriggerOffset; // in usec

      int fTriggerBin;
      int fBaselineMin;
      int fBaselineMax;

      vector<int> fpeakPositions;

      //define private functions and data members here
      

};

#endif /* VetoAnalysis_H */

///////////////////////////////////////////////////////////////////////////////// 
//Class Name: NoiseSelector
//Author:  M. Kos
//Description: This class select noise traces as done in PipeFitter code.
//  This is used by the PipeFitPhonon class to determine if a fit needs 
//  to be perfomred. 
// 
// Original authors of the PipeFitter code are L. Duong, J. Yoo, and E. Ramberg 
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef NoiseSelector_H
#define NoiseSelector_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the NoiseSelector Class.  
class NoiseSelector : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      NoiseSelector();  
      ~NoiseSelector(); //destructor 

      //Set parameters
      void InitializeParameters(); //do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse,const string &channelType, int zipnum, double RMS, double MaxADC);
      void DoCalc(map<string,double>& POFmap,map<string,double>& R20map, double ptThresh, double rdelThresh);

      //Return noise selection of pulse (TRUE = noise, FALSE = not noise)
      bool GetIsNoise() {return fIsNoise;}

   private:

      void ConstructRQList();

      
      //define private functions and data members here
      bool fIsNoise;
      bool fIsNoiseNew;
      bool fIsNoiseOld;
      bool IsNoisePhonon(const vector<double> &pulsevector,double thvalue=2000.0, double sigma = 3.,int zipnum = 1);
      bool IsNoiseCharge(const vector<double> &pulsevector,int lowbin=-1,int hibin=-1);
   

      //define constants here
      //FIXME - these are temporary and should be gotten from config
      int kLowerBin;
      int kUpperBin;
      double kThresArea;
      double kSigmaRMS;
      double fRMS;
      double fMaxADC;

};

#endif /* NoiseSelector_H */

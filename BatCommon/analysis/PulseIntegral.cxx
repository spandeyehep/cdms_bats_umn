/////////////////////////////////////////////////////////////////////// 
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

#include <iostream>

#include "PulseIntegral.h"
#include "PulseTools.h"
#include "PulseFilter.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
PulseIntegral::PulseIntegral() :
  fSampleRate(-999999.),
  fFilterCutoff(-999999.),
  fFilterOrder(-999999),
  fIntegral(-999999.)
{
   //   cout <<"Hello from PulseIntegral()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "PulseIntegral"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

PulseIntegral::~PulseIntegral()
{
//   cout <<"Goodbye from PulseIntegral()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void PulseIntegral::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("INTall", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//This is the main call for your analysis
void PulseIntegral::DoCalc(const vector<double>& aPulse, const string& pulseType)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"PulseIntegral::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(pulseType == "filtered" && (fFilterCutoff < 0 || fFilterOrder < 0 || fSampleRate < 0))
  {
    cerr<<"PulseIntegral::DoCalc ERROR! Filtered option was chosen, but filter parameters not set. "
	<< endl;
    exit(1);
  }

  //filter the pulse if that option was chosen
  vector<double> aWorkingPulse; //copy the pulse in case we want to filter it
  if(pulseType == "filtered")
    aWorkingPulse = PulseFilter::ButterLowPass(aPulse, fSampleRate, fFilterCutoff, fFilterOrder);
  else
    aWorkingPulse = aPulse;

   //simple integral of filtered pulse, we may want something slightly more sophisticated eventually 
  fIntegral = PulseTools::Area(aWorkingPulse);

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      fRQList["INTall"] = fIntegral;
   }

   return;
}

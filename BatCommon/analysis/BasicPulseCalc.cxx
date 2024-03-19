///////////////////////////////////////////////////////////////////////////////// 
//Class Name: BasicPulseCalc
//Authors: L. Hsu
//Description: This class performs some basic calculations on single pulses 
// (phonon or charge: baseline subtraction, normalization, calculate std 
//  baseline and pulse maximum
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//  Nov 2017: N. Mast: Add option to use sloped baseline subtraction
////////////////////////////////////////////////////////////////////////////////// 





#include <iostream>

#include "BasicPulseCalc.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
BasicPulseCalc::BasicPulseCalc(const string& sensorType) :
   fSatVal(999999.),  //set to high value for veto pulses
   fGain(-999999.),
   fBias(-999999.),
   fBiasTime(-999999.),
   fPulseNorm(1.),
   fBaselineMin(-999999),
   fBaselineMax(-999999),
   fSensorType(sensorType),
   fBaseline(-999999.),
   fBaselinePost(-999999.),
   fDoSlopedBsSub(false),
   fStd(-999999.),
   fIsSat(false),
   fNSat(-999999), 
   fMaximum(-999999.)
{
   //   cout <<"Hello from BasicPulseCalc()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "BasicPulseCalc"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

BasicPulseCalc::~BasicPulseCalc()
{
//   cout <<"Goodbye from BasicPulseCalc()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void BasicPulseCalc::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("bs", initVal));   //prepulse baseline
   fRQList.insert(pair<string,double>("bspost", initVal));  //end of pulse baseline
   fRQList.insert(pair<string,double>("sat", initVal));
   fRQList.insert(pair<string,double>("norm", initVal)); 
   fRQList.insert(pair<string,double>("std", initVal)); 
   fRQList.insert(pair<string,double>("gain", initVal)); //total gain
   fRQList.insert(pair<string,double>("bias", initVal));

   //only set biastime for charge channel
   if(fSensorType != "charge" && fSensorType != "phonon")
   {
     cout <<"ERROR BasicPulseCalc::ConstructRQList()  Unknown sensor type for this trace!"
	  << endl;
   }
   else
   {
     if(fSensorType == "charge")
       fRQList.insert(pair<string,double>("biastime", initVal)); 
   }

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//do it here and not in the constructor
void BasicPulseCalc::InitializeParameters()
{

   return;
}

//This is the main call for your analysis
void BasicPulseCalc::DoCalc(const vector<double>& aPulse)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"BasicPulseCalc::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(fBaselineMin == -999999 || fBaselineMax == -999999)
  {
    cerr <<"BasicPulseCalc::DoCalc ERROR! Baseline limits not set, please use SetBaselineRange() to do this."
	 << endl;
    exit(1);
  }

  //checking if pulse is saturated - currently not stored as an RQ, we save fNSat instead
  fIsSat = PulseTools::IsSaturated(aPulse, fSatVal);

  //checking the number of saturated bins
  fNSat = PulseTools::NumBinsSaturation(aPulse, fSatVal) + PulseTools::NumBinsMinimum(aPulse, fMinVal);

  //Offsetting fBaselineMin by 1 to account for for c++ element naming conventions
  //Pulse tools will loop from fBaselinMin to <fBaselineMax
  fBaselineMin -= 1;

  //getting mean baselines
  int nBins = aPulse.size();
  fBaseline = PulseTools::Baseline(aPulse, fBaselineMin, fBaselineMax);
  fBaselinePost = PulseTools::Baseline(aPulse, (nBins-fPostBaselineRange), nBins);

  //getting Std - this uses DarkPipe definitions and conventions!
  fStd = PulseTools::Std(aPulse, fBaselineMin, fBaselineMax)/fPulseNorm;

  //doing the baseline subtraction
  //Subtract the constant value of the prepulse baseline by default, or attempt to subtract a sloped baseline
  if(fDoSlopedBsSub){
    fBaselineSubPulse = PulseTools::SlopedBaselineSub(aPulse, fBaselineMin, fBaselineMax,fPostBaselineRange);
  }else{
    fBaselineSubPulse = PulseTools::BaselineSub(aPulse, fBaselineMin, fBaselineMax);
  }

  //doing normalization (norm = 1 if ISR is not read)
  fBaselineSubNormPulse = PulseTools::Normalize(fBaselineSubPulse, fPulseNorm);

  //Next, store the results of this calculation as the RQ's.
  //These values will be included in the output of BatRoot.
  if(fStoreRQs) {

    fRQList["bs"] = fBaseline;
    fRQList["bspost"] = fBaselinePost;
    fRQList["sat"] = fNSat;
    fRQList["norm"] = fPulseNorm;
    fRQList["std"] = fStd;
    fRQList["gain"] = fGain;
    fRQList["bias"] = fBias;

    if(fSensorType == "charge")
      fRQList["biastime"] = fBiasTime;
  }

  return;

}

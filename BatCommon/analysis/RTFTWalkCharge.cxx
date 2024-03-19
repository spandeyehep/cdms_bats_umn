/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: RTFTWalkCharge
//Authors: L. Hsu
//Description:  RTFT(RiseTimeFallTime)Walk algrorithm from darkpipe for charge 
//pulses.
//Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////



#include <iostream>

#include "RTFTWalkCharge.h"
#include "PulseTools.h"
#include "PulseFilter.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
RTFTWalkCharge::RTFTWalkCharge() :
   fPeakWindowMin(-999999), 
   fPeakWindowMinInitialized(false),
   fSampleRate(-999999.),
   fSampleRateInitialized(false),
   fSensorType(""),
   fSensorTypeInitialized(false),
   fFilterCutoff(-999999.),
   fFilterOrder(-999999),
   fStd(-999999),
   fMaxAmp(-999999.)
{
   //   cout <<"Hello from RTFTWalkCharge()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "RTFTWalkCharge"; 
   fStoreRQs = true;

   //initialize the sample points for RTFT walk, currently only have 20% time
   fPercentages.push_back(20); 

   //Construct the RQ list
   ConstructRQList();
}

RTFTWalkCharge::~RTFTWalkCharge()
{
//   cout <<"Goodbye from RTFTWalkCharge()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void RTFTWalkCharge::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   for(uint percentItr = 0; percentItr < fPercentages.size(); percentItr++)
   {
     //risetime
     string rqName = Form("r%d",fPercentages[percentItr]);
     fRQList.insert(pair<string,double>(rqName, initVal));
   }

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.
   fRQList.insert(pair<string,double>("max", initVal));

   return;
}

//This is the main call for your analysis
void RTFTWalkCharge::DoCalc(const vector<double>& aPulse, const string& pulseType)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"RTFTWalkCharge::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(!fSampleRateInitialized || !fSensorTypeInitialized || !fPeakWindowMinInitialized)
  {
    cerr<<"RTFTWalkCharge::DoCalc ERROR! Input parameters not initialized. "
	<< endl;
    exit(1);
  }

  if(pulseType == "filtered" && (fFilterCutoff < 0 || fFilterOrder < 0))
  {
    cerr<<"RTFTWalkCharge::DoCalc ERROR! Filtered option was chosen, but filter parameters not set. "
	<< endl;
    exit(1);
  }

  //filter the pulse if that option was chosen
  vector<double> aWorkingPulse; //copy the pulse in case we want to filter it
  if(pulseType == "filtered")
    aWorkingPulse = PulseFilter::ButterLowPass(aPulse, fSampleRate, fFilterCutoff, fFilterOrder);
  else
    aWorkingPulse = aPulse;

  //to go from physical time in units of bins to the correct c++ array position
  fPeakWindowMin -= 1;
     
  // ===== calculate max amplitude ====
  fMaxAmp = PulseTools::MaxADC(aWorkingPulse, fPeakWindowMin); 
  if(fStoreRQs) fRQList["max"] = fMaxAmp;

  // ===== calculate walk times, actual routine is in PulseTools =====
  for(uint timeItr=0; timeItr < fPercentages.size(); timeItr++)
  {
    double riseTime;
    int percent = fPercentages[timeItr];

    riseTime = PulseTools::RTFTWalkTimeQ(aWorkingPulse, (double)percent/100., fStd, fPeakWindowMin);

    //time is relative to start of digitized window (i.e. no trigger time offset)
    riseTime *= 1.0/fSampleRate;
    fRiseTimes[percent] = riseTime;

  
    //Next, store the results of this calculation as the RQ's.
    //These values will be included in the output of BatRoot.
    if(fStoreRQs) 
      fRQList[Form("r%d",percent)] = riseTime;


  }//end loop over percentages

  return;
}

/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: ConstFreqRTFTWalkPhonon
//Authors: L. Hsu
//Description:  RTFT(RiseTimeFallTime)Walk algrorithm from darkpipe for phonon 
//pulses.  Uses a single filtering frequency for all pulses on a given detector
//Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Feb. 2011 - RQ root names change from P*r% to P*WKr%. [LLH]
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#include <iostream>

#include "ConstFreqRTFTWalkPhonon.h"
#include "PulseTools.h"
#include "PulseFilter.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
ConstFreqRTFTWalkPhonon::ConstFreqRTFTWalkPhonon() :
   fPeakWindowMin(-999999), 
   fPeakWindowMax(-999999), 
   fPeakWindowMinInitialized(false),
   fPeakWindowMaxInitialized(false),
   fSampleRate(-999999.),
   fSampleRateInitialized(false),
   fSensorType(""),
   fSensorTypeInitialized(false),
   fFilterCutoff(-999999.),
   fFilterOrder(-999999)

{
   //   cout <<"Hello from ConstFreqRTFTWalkPhonon()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "ConstFreqRTFTWalkPhonon"; 
   fStoreRQs = true;

   //initialize the sample points for RTFT walk, current default is every 10%
   for(int itr=0; itr < 10; itr++)
   { fPercentages.push_back((itr+1)*10); }

   //add 95% starting with R130
   fPercentages.push_back(95);

   //Construct the RQ list
   ConstructRQList();
}

ConstFreqRTFTWalkPhonon::~ConstFreqRTFTWalkPhonon()
{
//   cout <<"Goodbye from ConstFreqRTFTWalkPhonon()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void ConstFreqRTFTWalkPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   for(int percentItr = 0; percentItr < (int) fPercentages.size(); percentItr++)
   {
     //risetime
     string rqName = Form("WKr%d",fPercentages[percentItr]);
     fRQList.insert(pair<string,double>(rqName, initVal));

     //falltime
     if(fPercentages[percentItr] == 80 || fPercentages[percentItr] == 40 || fPercentages[percentItr] == 20  ||
	fPercentages[percentItr] == 90 || fPercentages[percentItr] == 95)
     {
       string rqName = Form("WKf%d",fPercentages[percentItr]);
       fRQList.insert(pair<string,double>(rqName, initVal));
     }
   }

   //maximum of filtered pulse
   fRQList.insert(pair<string,double>("WKmax", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//This is the main call for your analysis
void ConstFreqRTFTWalkPhonon::DoCalc(const vector<double>& aPulse, const string& pulseType)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"ConstFreqRTFTWalkPhonon::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(!fSampleRateInitialized || !fSensorTypeInitialized || !fPeakWindowMinInitialized || !fPeakWindowMaxInitialized)
  {
    cerr<<"ConstFreqRTFTWalkPhonon::DoCalc ERROR! Input parameters not initialized. "
	<< endl;
    exit(1);
  }

  if(pulseType == "filtered" && (fFilterCutoff < 0 || fFilterOrder < 0))
  {
    cerr<<"ConstFreqRTFTWalkPhonon::DoCalc ERROR! Filtered option was chosen, but filter parameters not set. "
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
    
  // ===== calculate rise times, actual routine is in PulseTools =====

  for(uint timeItr=0; timeItr < fPercentages.size(); timeItr++)
  {
    double riseTime;
    double fallTime;
    int percent = fPercentages[timeItr];

//     cout <<"In ConstFreqRTFTWalkPhonon, peakwindowmin = " << fPeakWindowMin
// 	 <<"\nIn ConstFreqRTFTWalkPhonon, peakwindowmax = " << fPeakWindowMax
// 	 << endl;

    riseTime = PulseTools::RTFTWalkTimeP(aWorkingPulse, (double)percent/100., fPeakWindowMin, fPeakWindowMax);

    //time is relative to start of digitized window (i.e. no trigger time offset)
    riseTime *= 1.0/fSampleRate;
    fRiseTimes[percent] = riseTime;

    //Next, store the results of this calculation as the RQ's.
    //These values will be included in the output of BatRoot.
    if(fStoreRQs) 
      fRQList[Form("WKr%d",percent)] = riseTime;

    // ===== falltimes (only for select percentages) =====

    if(fPercentages[timeItr] == 80 || fPercentages[timeItr] == 40 || fPercentages[timeItr] == 20  ||
       fPercentages[timeItr] == 90 || fPercentages[timeItr] == 95)
    {    

      fallTime = PulseTools::RTFTWalkTimeP(aWorkingPulse, -(double)percent/100., fPeakWindowMin, fPeakWindowMax);

      //time is relative to start of digitized window (i.e. no trigger time offset)
      fallTime *= 1.0/fSampleRate;
      fFallTimes[percent] = fallTime;
      
      //Next, store the results of this calculation as the RQ's.
      //These values will be included in the output of BatRoot.
      if(fStoreRQs) 
	fRQList[Form("WKf%d",percent)] = fallTime;

    }//endif 95, 90, 80, 40 or 20 fallTimes

  }//end loop over percentages

  // store maximum of the pulse
  fRQList["WKmax"] =  PulseTools::MaxADC(aWorkingPulse, fPeakWindowMin,fPeakWindowMax);


  return;
}

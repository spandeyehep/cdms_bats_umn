/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: VarFreqRTFTWalkPhonon
//Authors: L. Hsu
//Description:  Variable Frequency RTFT(RiseTimeFallTime)Walk algrorithm 
//              for phonon pulses.
//              Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Jan. 2009 - This was made into the variable frequency walk by Scott Hertel
//Feb. 2011 - Renaming the rq's of the class to VWKr* to distiguish
//            from the ConstFreqRTFTWalk rq's, which are WK [LLH]
//Feb. 2011 - Renaming this class from RTFTWalkPhonon to 
//            VarFreqRTFTWalkPhonon.  RQ root names change from P*r%
//            to P*VWKr%. [LLH]
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#include <iostream>

#include "VarFreqRTFTWalkPhonon.h"
#include "PulseTools.h"
#include "PulseFilter.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
VarFreqRTFTWalkPhonon::VarFreqRTFTWalkPhonon() :
   fPeakWindowMin(-999999), 
   fPeakWindowMax(-999999), 
   fPeakWindowMinInitialized(false),
   fPeakWindowMaxInitialized(false),
   fSampleRate(-999999.),
   fSampleRateInitialized(false),
   fSensorType(""),
   fSensorTypeInitialized(false),
   fCutoffParameter1(-999999.),
   fCutoffParameter2(-999999.),
   fFilterOrder(-999999),
   fFilterCutoff(-999999.),
   fDoFilter(false),
   fMaxAmp(-999999.)
{
   //   cout <<"Hello from VarFreqRTFTWalkPhonon()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "VarFreqRTFTWalkPhonon"; 
   fStoreRQs = true;

   //initialize the sample points for RTFT walk, current default is every 10%
   for(int itr=0; itr < 10; itr++)
   { fPercentages.push_back((itr+1)*10); }

   //add 95% starting with R130
   fPercentages.push_back(95);

   //Construct the RQ list
   ConstructRQList();
}

VarFreqRTFTWalkPhonon::~VarFreqRTFTWalkPhonon()
{
//   cout <<"Goodbye from VarFreqRTFTWalkPhonon()" << endl;
}




void VarFreqRTFTWalkPhonon::SetVariableFilterParameters(int& channum,
						 vector<double> cutoff1, vector<double> cutoff2,
						 int order,
						 double lowerLimit,
						 double upperLimit,
						 double scalingFactor,
						 double pulseSTD,
						 double pulseOFeV,
						 double templateMax,
                                                 double defaultCutoff) 
{

   //assign order
   fFilterOrder = order;

   //check that the channel number is valid
   if(channum < 0 || (uint)channum > cutoff1.size() || (uint)channum > cutoff2.size())
   {
      cout <<"VarFreqRTFTWalkPhonon::ERROR!  Invalid channel number passed to SetVariableFilterParameters."
	   <<"There is no matching cutoff value for channel number = " << channum
	   << endl;
      exit(1);
   }
   
   //Retrieve functional form parameters
   fCutoffParameter1 = cutoff1[channum];
   fCutoffParameter2 = cutoff2[channum];

   //check that the parameters are valid 
   if(fCutoffParameter1 <= 0 || fCutoffParameter2 <= 0)
   {
      fFilterCutoff = defaultCutoff;
      return;
   }

   // Do Cutoff calculation
   // for reasonable pulses, th butterworth filter cutoff is a function of the signal to noise ratio
   fFilterCutoff = scalingFactor*fCutoffParameter2*(pow( templateMax*pulseOFeV/pulseSTD , fCutoffParameter1));	

//cout << "_________________________________________________________" << endl;
//cout << "fFilterCutoff = " << endl;
//cout << "     2 * " << fCutoffParameter2 << endl;
//cout << "       * [ " << templateMax << " * " <<pulseOFeV << " / " << pulseSTD << " ] ^ " << fCutoffParameter1 << endl;
//cout << "              = " << fFilterCutoff << endl;

    // if the calculated cutoff frequency is too high compared to the sampling rate, then don't filter at all
    if(fFilterCutoff >= upperLimit)
    {
      fFilterCutoff = upperLimit;
      return;
    }
    
    // if the calculated cutoff frequency is too low to be at all useful, set it to an artificial lower limit
    if(fFilterCutoff <= lowerLimit)
    {
       fFilterCutoff = lowerLimit;
    }
    
    // also if the signal height is negative, set the cutoff to the lower limit
    if(pulseOFeV <= 0)
    {
       fFilterCutoff = lowerLimit;
    }
    
//cout <<"calculated butterworth cutoff" << endl;
//cout <<fFilterCutoff<< endl;
	
    //turn on flag for filtering
    fDoFilter = true;

    return; 
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void VarFreqRTFTWalkPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   for(int percentItr = 0; percentItr < (int) fPercentages.size(); percentItr++)
   {
     //risetime
     string rqName = Form("VWKr%d",fPercentages[percentItr]);
     fRQList.insert(pair<string,double>(rqName, initVal));
     
     //falltime
     if(fPercentages[percentItr] == 80 || fPercentages[percentItr] == 40 || fPercentages[percentItr] == 20 ||
	fPercentages[percentItr] == 90 || fPercentages[percentItr] == 95)
     {
       rqName = Form("VWKf%d",fPercentages[percentItr]);
       fRQList.insert(pair<string,double>(rqName, initVal));
     }
	 
   }

   //the butterworth cutoff frequency that was used
   fRQList.insert(pair<string,double>("VWKCutoff", initVal));

   //maximum of filtered pulse
   fRQList.insert(pair<string,double>("VWKmax", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;

}

//This is the main call for your analysis
void VarFreqRTFTWalkPhonon::DoCalc(const vector<double>& aPulse)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"VarFreqRTFTWalkPhonon::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(!fSampleRateInitialized || !fSensorTypeInitialized || !fPeakWindowMinInitialized || !fPeakWindowMaxInitialized)
  {
    cerr<<"VarFreqRTFTWalkPhonon::DoCalc ERROR! Input parameters not initialized. "
	<< endl;
    exit(1);
  }

  if(fDoFilter && (fFilterCutoff < 0 || fFilterOrder < 0))
  {
    cerr<<"VarFreqRTFTWalkPhonon::DoCalc ERROR! Filtered option was chosen, but filter parameters not set. "
	<< endl;
    exit(1);
  }

  //filter the pulse if that option was chosen
  vector<double> aWorkingPulse; //copy the pulse in case we want to filter it
  if(fDoFilter)
    aWorkingPulse = PulseFilter::ButterLowPass(aPulse, fSampleRate, fFilterCutoff, fFilterOrder);
  else
    aWorkingPulse = aPulse;

  //to go from physical time in units of bins to the correct c++ array position
  fPeakWindowMin -= 1;

  // ===== calculate max amplitude =====
  fMaxAmp = PulseTools::MaxADC(aWorkingPulse, fPeakWindowMin,fPeakWindowMax); 
      
  // ===== calculate rise times, actual routine is in PulseTools =====

  for(uint timeItr=0; timeItr < fPercentages.size(); timeItr++)
  {
    double riseTime;
    double fallTime;
    int percent = fPercentages[timeItr];

    riseTime = PulseTools::RTFTWalkTimeP(aWorkingPulse, (double)percent/100., fPeakWindowMin, fPeakWindowMax);

    //time is relative to start of digitized window (i.e. no trigger time offset)
    riseTime *= 1.0/fSampleRate;
    fRiseTimes[percent] = riseTime;

    //Next, store the results of this calculation as the RQ's.
    //These values will be included in the output of BatRoot.
    if(fStoreRQs) 
      fRQList[Form("VWKr%d",percent)] = riseTime;

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
	fRQList[Form("VWKf%d",percent)] = fallTime;

    }//endif 95, 90, 80, 40 or 20 fallTimes

  }//end loop over percentages
  
  //some additional rq's to store
  if(fStoreRQs)
  {
    fRQList["VWKCutoff"] = fFilterCutoff;
    fRQList["VWKmax"] = fMaxAmp;
  }

  return;
}

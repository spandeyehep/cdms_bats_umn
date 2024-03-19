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


#include <iostream>

#include "VetoAnalysis.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
VetoAnalysis::VetoAnalysis()
{
   //   cout <<"Hello from VetoAnalysis()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "VetoAnalysis"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

VetoAnalysis::~VetoAnalysis()
{
//   cout <<"Goodbye from VetoAnalysis()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void VetoAnalysis::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("VT50Amp", initVal));
   fRQList.insert(pair<string,double>("VT50Time", initVal));
   fRQList.insert(pair<string,double>("VTTraceAmp", initVal));
   fRQList.insert(pair<string,double>("VTTraceTime", initVal));
   fRQList.insert(pair<string,double>("VTPreAmpFast", initVal));
   fRQList.insert(pair<string,double>("VTPreTimeFast", initVal)); 

   //fRQList.insert(pair<string,double>("VTTraceAmpSelect", initVal)); // for debugging only!
   fRQList.insert(pair<string,double>("VTNearPeakPreAmp", initVal));
   fRQList.insert(pair<string,double>("VTNearPeakPreTime", initVal));
   fRQList.insert(pair<string,double>("VTNumPeakPre", initVal));
//   fRQList.insert(pair<string,double>("VTMaxPeakPreAmp", initVal));  //often the same as NearPeak
//   fRQList.insert(pair<string,double>("VTMaxPeakPreTime", initVal)); //often the same as NearPeak

   fRQList.insert(pair<string,double>("VTbs", initVal));
   fRQList.insert(pair<string,double>("VTstd", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//optional function, do it here and not in the constructor
void VetoAnalysis::InitializeParameters()
{
  fSlope = -999999.;
  fSampleTime = -999999.;
  fBinToVolts = -999999.;
  fPreTime = -9999999.; //use minus seven 9's to distinguish from cases when its set to default value in history code
  fTriggerOffset = -999999.;

  fTriggerBin = -999999;
  fBaselineMin = -999999; 
  fBaselineMax = -999999; 

  return;
}

//This is the main call for your analysis
void VetoAnalysis::DoCalc(const vector<double>& aPulse)
{
  int i;
  double failvalue = -999999.0;
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"VetoAnalysis::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  if(fSampleTime == -999999. || fBinToVolts == -999999. || fTriggerBin == -999999 || fTriggerOffset == -999999.)
  {
    cerr <<"VetoAnalysis::DoCalc ERROR!  Veto information not initialized, please load SampleTime, BinToVolts, Trigger Bin and TriggerOffset!"
	 << endl;
    exit(1);
  }

  if(fBaselineMin == -999999 || fBaselineMax == -999999 || fSlope == -999999. || fPreTime == -9999999.)
  {
    cerr <<"VetoAnalysis::DoCalc ERROR!  Veto analysis parameters not initialized, please load Baseline range, Slope and PreTime!"
	 << endl;
    exit(1);
  }

  //quantities based on max adc found
  double vt50Amp = failvalue;
  double vt50Time = failvalue;
  double vtTraceAmp = failvalue;
  double vtTraceAmpSelect = failvalue;
  double vtTraceTime = failvalue;
  double vtPreAmpFast = failvalue;
  double vtPreTimeFast = failvalue;

  //quantities using peak finding algorithm
  double vtNearPeakPreAmp = failvalue;
  double vtNearPeakPreTime = failvalue;
  double vtNumPeakPre = failvalue;
  double vtMaxPeakPreAmp = failvalue;
  double vtMaxPeakPreTime = failvalue;

  //general quantities
  double vtbs = failvalue;
  double vtstd = failvalue;

  // ===== First, identify the biggest peak (max ADC value) in specified segments of the trace =====

  int nbins50 = fTriggerBin - (int)round(50.0/fSampleTime) - 1;
  //look past trigger time by this much for pulses since there appears to be an offset between the trigger bin and the true trigger time
  int nbinsExtend = (int)round(fTriggerOffset/fSampleTime); 
  double vetoBaseline = GetVetoBaseline(aPulse);

  //First find the biggest "peak" in the whole trace
  vtTraceAmp = (PulseTools::MaxADC(aPulse) - vetoBaseline)*fBinToVolts;
  vtTraceTime = (PulseTools::MaxADCPoint(aPulse) + 1 - fTriggerBin)*fSampleTime;

  //Next find the biggest "peak" in the 50 usec before the trigger, include the small offset time after the trigger, otherwise we will miss many veto hits
  vt50Amp = (PulseTools::MaxADC(aPulse, nbins50, fTriggerBin+nbinsExtend) - vetoBaseline)*fBinToVolts;
  vt50Time = (PulseTools::MaxADCPoint(aPulse, nbins50, fTriggerBin+nbinsExtend) + 1 - fTriggerBin)*fSampleTime;

  //Then find the biggest "peak" in a 10 usec window centered on VTPreTime 
  int nbins10 = (int)round(10.0/fSampleTime);
  
  //check for a valid pretime
  if(fPreTime != -999999)
  {
     int preTimeFastBin = (int)round(fPreTime/fSampleTime) + fTriggerBin - 1;
     int ptlowbin = (preTimeFastBin - nbins10);
     int pthibin = (preTimeFastBin + nbins10);
     
     //most of the time, VTPreTime is before the digitization begins so check this first
     if(preTimeFastBin > 0 && ptlowbin >= 0 && pthibin >= 0 && pthibin < (int)aPulse.size())
     {
	vtPreAmpFast = (PulseTools::MaxADC(aPulse, ptlowbin, pthibin) - vetoBaseline)*fBinToVolts;
	vtPreTimeFast = (PulseTools::MaxADCPoint(aPulse, ptlowbin, pthibin) + 1 - fTriggerBin)*fSampleTime;
     }
  } //end if valid pretime

  // ===== Next, use the pipefitter peak finding algorithm to identify peaks and store them ======

  // use peak finding algorithm to retrieve vector of found peaks   
  vector<int> peakPos = GetPeakPos(aPulse); //vector of peak positions in vector bins
  double tempMaxTraceAmp = failvalue;
  vector<int> beforeTriggerTimes;
  vector<int> afterTriggerTimes;

//  cout <<"\nFresh pulse! "<< endl;
  
  //loop over found peaks to get maximum
  //also sort the peak times into before and after trigger time vectors
  for(i=0;i<(int) peakPos.size();i++)
  {
     // finding maximum peak in the whole trace - for debugging
     if( GetPeakAmp(aPulse,peakPos[i])>tempMaxTraceAmp ) 
     {
	tempMaxTraceAmp = GetPeakAmp(aPulse,peakPos[i]); 
     }

     // finding maximum peak in the trace before the trigger time
     // include the trigger offset, otherwise we will miss many veto hits that actually occur at the trigger time
     if(peakPos[i] < (fTriggerBin + nbinsExtend -1) && GetPeakAmp(aPulse,peakPos[i])> vtMaxPeakPreAmp)
     {
	vtMaxPeakPreTime = (peakPos[i] + 1 - fTriggerBin)*fSampleTime;
	vtMaxPeakPreAmp = GetPeakAmp(aPulse,peakPos[i]);
     }

     // saving these to separate vectors for later convenience 
     // include the trigger offset, otherwise we will miss many veto hits that actually occur at the trigger time
     if(peakPos[i] > fTriggerBin+nbinsExtend-1) afterTriggerTimes.push_back(peakPos[i]);
     if(peakPos[i] <= fTriggerBin+nbinsExtend-1) beforeTriggerTimes.push_back(peakPos[i]);

  } // end loop over found peaks

  //if we found a (max) peak at all, scale time and adjust baseline
  if(tempMaxTraceAmp > failvalue)
  {
     vtTraceAmpSelect = (tempMaxTraceAmp - vetoBaseline)*fBinToVolts;
  }
  if(vtMaxPeakPreAmp > failvalue)
  {
     vtMaxPeakPreAmp = (vtMaxPeakPreAmp - vetoBaseline)*fBinToVolts;
  }

  //Get the latest peak that is closest to the trigger time
  if(beforeTriggerTimes.size() > 0)
  {
    int nearBin = -999999;

    //iterate backwards to find closest time after fTriggerBin
    for(int backItr = beforeTriggerTimes.size()-1; backItr >= 0; backItr--)
    {
      int testBin = beforeTriggerTimes[backItr];
      
      if(testBin+1 >= fTriggerBin) 
	nearBin = testBin;
      
      if(testBin+1 < fTriggerBin)  
      {
	//if no post fTriggerBin times found then take the most recent one before the fTriggerBin
	if(nearBin == -999999) nearBin = testBin;
	break;
      }
    }

    vtNearPeakPreTime = (nearBin + 1 - fTriggerBin)*fSampleTime;  
    vtNearPeakPreAmp = (GetPeakAmp(aPulse, nearBin) - vetoBaseline)*fBinToVolts; 
  }

  //store the number of peaks
  vtNumPeakPre = beforeTriggerTimes.size();


//   for(int beforeItr=0; beforeItr < beforeTriggerTimes.size(); beforeItr++)
//   {
//      cout <<"before peak pos = " << beforeTriggerTimes[beforeItr] 
// 	  << endl;     
//   }

//   cout <<"vtNearPeakPreTime = " << vtNearPeakPreTime
//        <<"\nnpeaks before = " << beforeTriggerTimes.size()
//        <<"\nmax peak before = " << vtMaxPeakPreAmp
//        <<"\nmax peak time before = " << vtMaxPeakPreTime
//        << endl;

  // note the following few lines below give the same result as DarkPipe
  // so, instead we are using Pipefitter baseling finding algorithm
  // === calculate the baseline of the first 70musec using standard method (in PulseTools) ===
  //   vtbs = PulseTools::Baseline(aPulse,0,nbins70);
  vtbs = vetoBaseline*fBinToVolts-5.0; //convert to volts and then subtract 5V (darkpipe)
  
  // === calculate the standard deviation using standard method (in PulseTools) ===
  // FIXME! this doesn't agree with DarkPipe values, so consider implementing something else
  int nbins70 = (int)round(70.0/fSampleTime);
  vtstd = fBinToVolts*PulseTools::Std(aPulse,0,nbins70);

  // === Next, store the results of this calculation as the RQ's. ===

  //These values will be included in the output of BatRoot.
  if(fStoreRQs) 
  {
    fRQList["VT50Amp"] = vt50Amp;
    fRQList["VT50Time"] = vt50Time; 
    fRQList["VTTraceAmp"] = vtTraceAmp;
    fRQList["VTTraceTime"] = vtTraceTime; 
    fRQList["VTPreAmpFast"] = vtPreAmpFast;
    fRQList["VTPreTimeFast"] = vtPreTimeFast; 

    //fRQList["VTTraceAmpSelect"] = vtTraceAmpSelect; //for debugging only!!
    fRQList["VTNearPeakPreAmp"] = vtNearPeakPreAmp;
    fRQList["VTNearPeakPreTime"] = vtNearPeakPreTime;
    fRQList["VTNumPeakPre"] = vtNumPeakPre;
//    fRQList["VTMaxPeakPreAmp"] = vtMaxPeakPreAmp; //extra info
//    fRQList["VTMaxPeakPreTime"] = vtMaxPeakPreTime; //extra info

    fRQList["VTbs"] = vtbs;
    fRQList["VTstd"] = vtstd;


  }  //Done with RQ storage
 
 return;
}

double VetoAnalysis::GetVetoBaseline( const vector<double>& trace ) const
{

  int offset = fBaselineMin, vetoProjection[1000] = { 0 }; // Offset window and diff
  int heightA = 0, heightB = 0, heightC = 0;       // Magnitude of 3 heights
  int occA = 0, occB = 0, occC = 0;                // Frequency of 3 heights
  int i;

  // Search between window of offset < x < offset + 1000
  for( i = 0; i < (int) trace.size(); i++ )
    {
      if( trace.at(i) > offset && trace.at(i) < fBaselineMax )
	{
	  vetoProjection[(int)trace.at(i) - offset]++;
	}
    }

  // Find 3 most common heights and their frequencies
  for( i = 0; i < 1000; i++ )
    {
      if( vetoProjection[i] > occA )
	{
	  heightC = heightB;
	  heightB = heightA;
	  heightA = i;
	  occC = occB;
	  occB = occA;
	  occA = vetoProjection[i];
	}
    }

  // Take the weighted average and call it the baseline
  double base = double( heightA * occA + heightB * occB + heightC * occC ) / double( occA + occB + occC );

  //  cout <<"baseline = " << base + offset << endl;

  return double( base + offset );

}

//returns vector of bins where a peak was found
vector<int> VetoAnalysis::GetPeakPos( const vector<double>& trace ) const
{

   vector<double> vetoDiff;
   vector<int> peakPos; // Differential and position
   uint i;

  // Find the differential at each bin
  for( i = 0; i < trace.size() - 1; i++ )
  {
     vetoDiff.push_back(trace.at(i + 1) - trace.at(i));
  }

  // Locate peak based on fSlope change from + to - and 
  // check for minimum rise of the + slope
  for( i = 3; i < vetoDiff.size(); i++ )
  {
     if( vetoDiff.at(i) <= 0 && vetoDiff.at(i - 1) > 0 &&
	 ( vetoDiff.at(i - 1) > fSlope ||
	   vetoDiff.at(i - 2) > fSlope ||
	   vetoDiff.at(i - 3) > fSlope ) )
     {
//	cout <<"Found a peak at adc position i = " << i << endl;
	peakPos.push_back(i);
     }
  }

  return peakPos;
}

double VetoAnalysis::GetPeakAmp(const vector<double>& vetovector, int peakPos ) const
{
  return vetovector[peakPos];
}


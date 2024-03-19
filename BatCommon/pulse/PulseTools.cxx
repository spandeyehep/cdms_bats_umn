//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: PulseTools
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  A class containing functions for obtaining various quantities from   
//the pulses as well as FFT functions.  These functions are used by the analysis classes.    
//All member functions are declared as static so that this class does not need to be initialized.
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//  N. Mast, Nov 14, 2017:  Add sloped baseline subtraction function: SlopedBaselineSub
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
#include <iomanip>

#include "TRandom3.h"
#include "TFile.h"

#include "PulseTools.h"
 
using namespace std;

//======================================================================================

bool PulseTools::IsSaturated(const vector<double> &pulsevector,double satvalue)
{
  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::IsSaturated - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double maxv = MaxADC(pulsevector);
  if (maxv >= satvalue) return 1;
  else return 0;
}

//==================================================================================

template<class Type>
Type PulseTools::pulseDot(const vector<Type> &veca,const vector<Type> &vecb)
{
  Type out=0.0;
  if(veca.size() != vecb.size())
    return out;
  for(int i=0;i<(int)veca.size();i++)
    out+=veca[i]*vecb[i];

  return out;
}

//==================================================================================

template<class Type>
vector<Type> PulseTools::pulseNSMult(const vector<Type> &veca,const vector<Type> &vecb)
{
  vector<Type> out;
  if(veca.size() != vecb.size())
    return out;
  for(int i=0;i<(int)veca.size();i++)
    out.push_back(veca[i]*vecb[i]);

  return out;
}

//==================================================================================

template<class Type>
vector<Type> PulseTools::pulseScale(const vector<Type> &vec,Type scale)
{
  vector<Type> out;
  for(int i=0;i<(int)vec.size();i++)
    out.push_back(vec[i]*scale);

  return out;
}

//==================================================================================

vector<double> PulseTools::Normalize(const vector<double> &pulsevector, const double& norm)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::Normalize - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   vector<double> rvector;

   for (uint i=0; i < pulsevector.size(); i++) {
      rvector.push_back(pulsevector[i]/norm);
   }

   return rvector;
}

//==================================================================================

vector<double> PulseTools::Scale(const vector<double> &pulsevector, const double& scale)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::Scale - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   vector<double> rvector;

   for (uint i=0; i < pulsevector.size(); i++) {
      rvector.push_back(pulsevector[i]*scale);
   }

   return rvector;
}

//==================================================================================

vector<double> PulseTools::InvertPulse(const vector<double> &pulsevector)
{
  int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::InvertPulse - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   return Normalize(pulsevector, -1.0);
}

//==================================================================================

vector<double> PulseTools::SumPulses(const vector<double> &pulsevector1, const vector<double> &pulsevector2)
{
   if(pulsevector1.size() == 0 || pulsevector2.size() == 0)
   {
      cout <<"PulseTools::SumPulses ERROR!  Attempting to add two pulses, but one or both pulses are of size 0!" << endl;
      exit(1);
   }

   if(pulsevector1.size() != pulsevector2.size())
   {
      cout <<"PulseTools::SumPulses ERROR!  Attempting to add two pulses of different length!" << endl;
      exit(1);
   }

   vector<double> sumPulse = pulsevector1;
   for(uint adcCtr=0; adcCtr<pulsevector1.size(); adcCtr++)
   {
      sumPulse[adcCtr] += pulsevector2[adcCtr];
   }

   return sumPulse;
}

//===========================================================================================

double PulseTools::Area(const vector<double> &pulsevector,int lowbin, int hibin){

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::Area - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  if(hibin > nBins)
  {
    cerr <<"PulseTools::Area - ERROR! interval passed to this function exceeds pulse length" << endl;
    exit(1);
  }



  int lbin=0, hbin=0, i;
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;

  double adcarea = 0.;
  for (i=lbin; i<hbin; i++) {
    adcarea += pulsevector[i];
  }
  return adcarea;
}

//===========================================================================================

double PulseTools::Baseline(const vector<double> &pulsevector,int lowbin,int hibin)
{
  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::Baseline - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double ped = 0.;
  int     count = 0;
  int lbin=0, hbin=0,i;

  vector<double> rvector;
  if (lowbin < 0) lbin = 0;
  if (hibin < 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin; 

  for (i=lbin; i<hbin; i++) {
    ped  += pulsevector[i];
    count ++;
  }
  ped = ped/(double)count;
  return ped;
}

//===========================================================================================

vector<double> PulseTools::BaselineSub(const vector<double> &pulsevector, int lowbin,int hibin)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::BaselineSub - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double ped = 0.;
  int     count = 0;
  int lbin=0, hbin=0,i;

  vector<double> rvector;
  if (lowbin < 0) lbin = 0;
  if (hibin < 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin; 

  for (i=lbin; i<hbin; i++) {
    ped  += pulsevector[i];
    count ++;
  }
  ped = ped/(double)count;
  
  for (i=0; i<nBins; i++) {
    rvector.push_back(pulsevector[i]-ped);
  }
  
  return rvector;
  
}

//===========================================================================================

//Calculate baseline slope in [ADC/bin]
double PulseTools::SlopedBaseline(const vector<double> &pulsevector, int lowBin,int hiBin, int endBins)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::BaselineSub - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double pre = 0., post=0.;
  int countPre = 0, countPost=0;
  double m=0., b=0.;
  int i;

  vector<double> rvector;
  if (lowBin < 0) lowBin = 0;
  if (hiBin < 0) hiBin = nBins;
  if (endBins < 0) endBins = nBins;

  //Get prepulse average
  for (i=lowBin; i<hiBin; i++) {
    pre  += pulsevector[i];
    countPre ++;
  }
  pre = pre/(double)countPre;
  
  //Get postpulse average
  for (i=nBins-endBins; i<nBins; i++) {
    post  += pulsevector[i];
    countPost ++;
  }
  post = post/(double)countPost;
  
  //Calculate slope and offset
  m=(post-pre)/((double)nBins-(double)lowBin-((double)(countPre+countPost))/2.);
  b=pre-m*((double)lowBin+((double)countPre)/2.);

  return m;
  
}


//===========================================================================================

//Subtracts downward sloped baselines, or constant prepulse baseline if slope is not downwards
vector<double> PulseTools::SlopedBaselineSub(const vector<double> &pulsevector, int lowBin,int hiBin, int endBins)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::BaselineSub - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double pre = 0., post=0.;
  int countPre = 0, countPost=0;
  double m=0., b=0.;
  int i;

  vector<double> rvector;
  if (lowBin < 0) lowBin = 0;
  if (hiBin < 0) hiBin = nBins;
  if (endBins < 0) endBins = nBins;

  //Get prepulse average
  for (i=lowBin; i<hiBin; i++) {
    pre  += pulsevector[i];
    countPre ++;
  }
  pre = pre/(double)countPre;
  
  //Get postpulse average
  for (i=nBins-endBins; i<nBins; i++) {
    post  += pulsevector[i];
    countPost ++;
  }
  post = post/(double)countPost;
  
  //Calculate slope and offset
  m=(post-pre)/((double)nBins-(double)lowBin-((double)(countPre+countPost))/2.);
  b=pre-m*((double)lowBin+((double)countPre)/2.);

  //If the slope is negative, subtract the linear baseline fit, otherwise just
  //subtract the constant prePulse baseline

  if(m<0){
    for (i=0; i<nBins; i++) rvector.push_back(pulsevector[i]-(m*i+b));
  }else{
    for (i=0; i<nBins; i++) rvector.push_back(pulsevector[i]-pre);
  }
  
  return rvector;
  
}
//==========================================================================================

double PulseTools::MaxADC(const vector<double> &pulsevector,int lowbin, int hibin)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::MaxADC - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  int lbin=0, hbin=0,i;
  double maxadc = -999999.; 

  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;
  for (i=lbin; i<hbin; i++) {
    if (pulsevector[i] > maxadc) {
      maxadc = pulsevector[i];
      }
  }
  return maxadc;
}

//=============================================================================================

double PulseTools::MaxADCPoint(const vector<double> &pulsevector,int lowbin, int hibin)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::MaxADCPoint - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }


  int lbin=0, hbin=0,i;
  double maxadc = -999999.;
  double maxadcpt = -999999;
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;
  for (i=lbin; i<hbin; i++) {
    if (pulsevector[i] > maxadc) {
      maxadc = pulsevector[i];
      maxadcpt = (double) i;
      }
  }
  return maxadcpt;
}

//============================================================================================
//Return difference between maximum and minimum point in a pulse
//Note this function returns float to avoid subtle rounding issues when the value 
//is a true integer
float PulseTools::MinMax(const vector<double> &pulsevector,int lowbin, int hibin)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::MinMax - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  int lbin=0, hbin=0,i;
  double maxadc = -999999.;
  double minadc = pulsevector[0];
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;


  for (i=lbin; i<hbin; i++) {
    if (pulsevector[i] > maxadc) {
      maxadc = pulsevector[i];
    }
    if (pulsevector[i] < minadc){
      minadc = pulsevector[i];
    }
  }
  return (float)(maxadc-minadc);
}


minmax_struct PulseTools::MinMaxStruct(const vector<double> &pulsevector,int lowbin, int hibin)
{
  
  int nBins = pulsevector.size();
  if(nBins == 0)
    {
      cerr <<"PulseTools::MinMax - ERROR! empty pulse passed into this function" << endl;
      exit(1);
    }

  int lbin=0, hbin=0,i;
  double maxadc = -999999.;
  double maxadcpt = -999999;
  double minadc = pulsevector[0];
  double minadcpt = 0;
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;
  for (i=lbin; i<hbin; i++) {
    if (pulsevector[i] > maxadc) {
      maxadc = pulsevector[i];
      maxadcpt = (double) i;
    }
    if (pulsevector[i] < minadc){
      minadc = pulsevector[i];
      minadcpt = (double) i;
    }
  }
  
  minmax_struct minmaxData;
  minmaxData.max = maxadc;
  minmaxData.max_bin = maxadcpt;
  minmaxData.min = minadc;
  minmaxData.min_bin = minadcpt;
  minmaxData.minmax = maxadc-minadc;
 
  return minmaxData;
}





//============================================================================================
//Return change in baseline between beginning and end of pulse divided by noise
double PulseTools::PileUp(const vector<double> &pulsevector,int ndiv)
{
  int i;
  int nbins = pulsevector.size();
  if(nbins == 0)
    {
      cerr <<"PulseTools::PileUp - ERROR! empty pulse passed into this function" << endl;
      exit(1);
    }
  if(ndiv % 2 != 0) 
    {
      cerr <<"PulseTools::PileUp - ERROR! Number of divisions must be multiple of 2" << endl;
      exit(1);
    }

  //Get baseline at beginning of pulse and end of pulse
  double baseline1 = Baseline(pulsevector,0,(int) nbins/ndiv);
  double baselinelast = Baseline(pulsevector,(int) (nbins-nbins/ndiv),nbins);
  //Now get minimum std in pulse for all ndiv chunks
  double tmpstd = 9999.0;
  double stdev;
  for (i=1;i<ndiv;i++){
    stdev = Std(pulsevector,i*((int) nbins/ndiv),(i+1)*((int) nbins/ndiv));
    if(stdev < tmpstd) tmpstd = stdev;
    if(tmpstd == 0.) tmpstd = 1.;
  }
  double pcut = (baseline1-baselinelast)/tmpstd;

  
  return pcut;
  
}


//============================================================================================
//Note, this is identical to the Std routine, except for the normalization and the name.  These are
//being preserved as a separate routine for convenience of reproducting PipeFitter conventions
//Another Note:  the rather inefficient means of looping twice is to circumvent rounding issues 
//associated with the alternate calculation where the ped and rms are simultaneously calculated
double PulseTools::RMS(const vector<double> &pulsevector,int lowbin,int hibin)
{

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::RMS - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double ped = 0.;
  double rms = 0.;
  double  sum2 = 0.;
  int     count = 0;
  int lbin=0, hbin=0,i;

  //setting window of calculation
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;

  //first calc ped
  for (i=lbin; i<hbin; i++) {
    ped  += pulsevector[i];
    count ++;
  }
  ped = ped/(double)count;

  //now calc rms
  for (i=lbin; i<hbin; i++) 
  { 
     sum2 += (pulsevector[i] - ped)*(pulsevector[i] - ped);
  }
  rms = sqrt(sum2/(double)count);

  return rms;
}

//============================================================================================
//Note, this is identical to the above routine, except for the normalization and the name.  These are
//being preserved as a separate routine for convenience of reproducting Darkpipe conventions
//Another Note:  the rather inefficient means of looping twice is to circumvent rounding issues 
//associated with the alternate calculation where the ped and std are simultaneously calculated
double PulseTools::Std(const vector<double> &pulsevector,int lowbin,int hibin){

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::RMS - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double ped = 0.;
  double std = 0.;
  double sum2 = 0.;
  double count = 0.;
  int lbin=0, hbin=0,i;

  //setting window of calculation
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nBins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin;

  //first calc ped
  for (i=lbin; i<hbin; i++) {
    ped  += pulsevector[i];
    count ++;
  }
  ped = ped/(double)count;

  //now calc rms
  for (i=lbin; i<hbin; i++) 
  { 
     sum2 += (pulsevector[i] - ped)*(pulsevector[i] - ped);
  }

  std = sqrt(1.0/(count-1.0))*sqrt(sum2);

  return std;
}

//============================================================================================

//Rise time and fall time walk time.  Taken from cdmspipe, Richard Schnee's code
double PulseTools::RTFTWalkTimeP(const vector<double> &pulsevector,double percent_peak, int lowbin, int hibin){

  int nBins = pulsevector.size();
  if(nBins == 0)
  {
    cerr <<"PulseTools::RTFTWalkTimeP - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  int i,foundbin;
  double maxvalue =  MaxADC(pulsevector, lowbin, hibin);
  int maxbin = (int) MaxADCPoint(pulsevector, lowbin, hibin);
  double ref = percent_peak*maxvalue;
  double time_out = 0.;  
  int startBin = (lowbin > 0 ? lowbin: 0);

  foundbin = -1;

  //  if(percent_peak == 1) 
  // {
  // cout <<"\nbefore: maxbin = " << maxbin 
  //	 <<"\nstartBin = " << startBin <<", hibin = " << hibin
  //	 << endl;
  //  }

  // =====  calculating risetimes =====

  if (percent_peak >= 0.)
  {
    //walk down in adc bins until you go past the ref value
    for(i=maxbin; i>=startBin; i--)
    {      
      //note if ref < 0, then maxvalue is negative and walk will fail
      if(pulsevector[i] < ref) 
      {
        foundbin = i;
	i=-1;
      }
    }      

    //to setup for a very crude extrapolation - this can fail miserably sometimes to give a physical value
    if(foundbin < 0)   foundbin = startBin;  

    //interpolate - can foundbin == last bin ever?
    if( (foundbin >= startBin) && ((unsigned int)foundbin < (pulsevector.size()-1)) && ref > 0) 
    {     
      //to protect against zero denominator in the interpolation 
      if (pulsevector[foundbin+1] == pulsevector[foundbin]) 
      {
	time_out = 1+foundbin;
      } 
      else 
      {
	time_out = (double) (1+foundbin) + (ref-pulsevector[foundbin])/(pulsevector[foundbin+1]-pulsevector[foundbin]);
      }
    }   //end interpolation
    else
    { time_out = 0.0; }   //case of failed walk

  }  //end calcuation of risetimes

  //  if(percent_peak == 1) cout <<"after: maxbin = " << time_out-1 << endl;
  //if((time_out-1 != maxbin) && percent_peak ==1) cout <<"!!!!!!!!!!!!!!!! before and after maxbins are not the same!!!!!" << endl;

  // ===== calculating falltimes =====

  if (percent_peak < 0.)
  {
    ref = -1.0*ref; //this will be positive for normal pulses

    //walk *up* in adc bins, but down the falling side of the pulse
    for(i=maxbin;(unsigned int)i < pulsevector.size(); i++)
    {      
      if(pulsevector[i] < ref) 
      {
        foundbin = i;
	i=pulsevector.size()+1;
      }
    }

    //interpolate (note ref should be a positive number, if ref is < 0 then foundbin will be = maxbin)
    if( foundbin > maxbin ) 
    {     
      time_out = (double) (1+foundbin) - (ref-pulsevector[foundbin])/(pulsevector[foundbin-1]-pulsevector[foundbin]);  
    }    
    //this is called if you don't find the specified fall time (i.e. the walk failed) or in the case where the maxvalue is negative
    else
    {
      time_out = 0.0; 
    }

//     if(time_out > pulsevector.size() || time_out < 0.0 )
//       cout <<"\nunphysical bin time! = " << time_out
// 	   <<"foundbin = " << foundbin <<", ref = " << ref <<", pulsevector[foundbin] = " << pulsevector[foundbin] <<", pulsevector[foundbin - 1] " << pulsevector[foundbin-1] 
// 	   <<"\nmaxvalue = " << maxvalue <<", maxbin = " << maxbin <<", percent = " << percent_peak
// 	   <<"\n startBin = " << startBin
// 	   << endl;

  } //end falltime calc	

  return time_out; 

}

//============================================================================================
//
//       Rise and fall walk times for charge pulses - Taken from cdmspipe
//       Uses Paul Brink's simple algorithm (which 'walks' up from the 50% rise 
//       point to the RThigh point, walks down from 50% to the RTlow point, and
//       similarly walks from the peak down to the .37 point to find the ft.
// 
//       start   = vector of start times (in bins), last time below RTlow
//       rt      = vector of rise times (in bins), time between RTlow point
//                               and RThigh point
//       ft      = vector of fall times (in bins), time between peak and .37//peak
//
//       Traces  = matrix of background-subtracted, possibly filtered traces
//                               (mbins rows by ntraces columns)
//       STthresh = scalar value for start time threshold
//       RTlow   = scalar value indicating point from which to start measuring 
//                               risetime (e.g. 0.1 for 10%-90% rise)
//       RThigh  = scalar value indicating point at which to stop measuring 
//                               risetime (e.g. 0.9 for 10%-90% rise)
//
//                                       1998 June 20   Richard Schnee
//       Ported from DarkPipe verbatim.  Could use some revamping
//                                       2008 Dec  08   LLH
// ===========================================================================================

double PulseTools::RTFTWalkTimeQ(const vector<double> &pulsevector,double percent_peak, double stdev, 
				 int lowbin, int hibin)
{
  int nBins = pulsevector.size();

  if(nBins == 0)
  {
    cerr <<"PulseTools::RTFTWalkTimeQ - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  double time_out = 0.; 

  // === use the RTFTWalkTimeP for fall time ===

  if (percent_peak < 0.) time_out = RTFTWalkTimeP(pulsevector, percent_peak, lowbin, hibin);

  // ====== otherwise, use the following for rise time ======

  // === use a truncated version of the pulse from [lowbin, hibin] ===
  
  vector<double> truncatedPulse;
  int startBin = ( lowbin > 0 ? lowbin : 0);
  int endBin = ( hibin > 0 ? hibin : pulsevector.size());

  for(int binCtr = startBin; binCtr < endBin; binCtr++)
  {
     truncatedPulse.push_back(pulsevector[binCtr]);
  } 

  // === Start calculation of max adc bin ===

  double maxvalue =  MaxADC(truncatedPulse);
  unsigned int maxbin = (unsigned int) MaxADCPoint(truncatedPulse);
  //double ref = percent_peak*maxvalue;
  double tempsum; 
  int i,foundbin,num,j;
  foundbin = -1;

  if (maxvalue < (6.5 * stdev))
  {
     maxvalue = 0;
     maxbin = 0;
     num = truncatedPulse.size()/12; //rounds down
     
     // === First go through 12-sample chunks, and choose the one with the highest mean. ===

     for (i=0; i<num; i++)
     {
	tempsum = 0.;
	for (j=i*12; j<(i+1)*12; j++)
	{
	   tempsum += truncatedPulse[j];
	}
	if ((tempsum > maxvalue) || (maxbin == 0))
	{
	   maxvalue = tempsum;
	   maxbin = i*12 + 6; //maxbin = bin in the middle of this 12-sample chunk
	}
     }// end loop over chunks
     
     // === If there are odd samples left over (tlength not a multiple of 12), use the last 12 samples as another chunk. ===
     
     if ((truncatedPulse.size() - num*12) > 0.)
     {
	tempsum = 0;
	for (j=(int)truncatedPulse.size()-12; j<(int)truncatedPulse.size(); j++)
	{
	   tempsum += truncatedPulse[j];
	}
	if ((tempsum > maxvalue) || (maxbin == 0))
	{
	   maxvalue = tempsum;
	   maxbin = truncatedPulse.size() - 6;
	}
     } // end loop over ending chunk
     
     // Now something a little strange, but inherited: 
     // Look at the 24 samples around the center of the highest-mean
     // 12-sample chunk, and choose the highest sample.  If the chunk 
     // is too close to either end of the array, use only 6 samples. 
     
     if( maxbin<24 || maxbin > (truncatedPulse.size()-25) ) 
     {
	num=6;
     }
     else
     {
	num=24;
     }
     
     maxvalue = truncatedPulse[maxbin-num];
     foundbin = maxbin-num;
     for (i=(int)maxbin-num+1; i<(int)maxbin+num; i++)
     {
	if (truncatedPulse[i] > maxvalue)
	{
	   maxvalue=truncatedPulse[i];
	   foundbin=i;
	}
     }
     maxbin = foundbin;

  } //end if maxvalue < 6.5*stdev

  // ====== Done with routine for finding maxval, now do the walk =====
  
  double ref = percent_peak * maxvalue;
  foundbin = 0;
  
  for(i=maxbin;i>=0;i--){      
     if(truncatedPulse[i] < ref) {
        foundbin = i;
	i=-1;
     }
  }      
  
  if ((foundbin >= 0) && ((unsigned int)foundbin < truncatedPulse.size()-1)) {     /* interpolate   */

     if (truncatedPulse[foundbin+1] == truncatedPulse[foundbin])
     {
	time_out = 1+foundbin;
     } 
     else {
	time_out = (double) (1+foundbin) + (ref-truncatedPulse[foundbin])/(truncatedPulse[foundbin+1]-truncatedPulse[foundbin]);
     }
  }
  
  if ((unsigned int) foundbin == truncatedPulse.size()-1) {
     if (truncatedPulse[foundbin] == 0.)
     {
	time_out = 1+foundbin;
     } else {
	time_out = 1+foundbin + (ref-truncatedPulse[foundbin])/(0.-truncatedPulse[foundbin]);
     }
  }

  //readjust for the truncation
  time_out += startBin;

  return time_out;
}

//===========================================================================================
//PipeFitter calculation of walktime, used to make sure that all PipeFitPhonon values agree with PipeFitter
//FIXME we need to stop using this function for production...
double PulseTools::PipeFitterWalk(const vector<double> &pulsevector,double testvalue, 
				  int startbin, int endbin)
{
  int i;
  int threshold_bin = 499;//initialize to 500 as default because
                          //most risefunction start times are around 500
  for (i=startbin; i<endbin; i++){
    if (pulsevector[i] > testvalue) {
      threshold_bin = i;
      break;
    }
  }
  return (double)threshold_bin;
}

//===========================================================================================
//using darkpipe symmetric convention for normalization
void PulseTools::RealToComplexFFT(const vector<double>& pulsevector, vector<TComplex>& outComp)
{
    int n = pulsevector.size();
    if(n == 0)
    {
      cerr <<"PulseTools::RealToComplexFFT - ERROR! empty pulse passed into this function" << endl;
      exit(1);
    }

    double *pvector = new double[pulsevector.size()];
    double re,im;
    int i;

   //copying std vector back into array to pass to FFT (to mirror what is done in PulseTools)
   for (i=0;i<(int)pulsevector.size();i++){
      pvector[i] = pulsevector[i];  
   }

   TVirtualFFT *fftr2c = TVirtualFFT::FFT(1,&n,"R2C ES");
   fftr2c->SetPoints(pvector);
   fftr2c->Transform();
   fftr2c->GetPointComplex(0,re,im);
   TComplex tempCopy(re/sqrt((double)n), im/sqrt((double)n));
   outComp.push_back(tempCopy);

   for(i=1; i<n ;i++)
   {
      fftr2c->GetPointComplex(i,re,im);
      TComplex tempCopy(re/sqrt((double)n), im/sqrt((double)n));
      outComp.push_back(tempCopy);
   }

   //done! so delete new'd objects
   delete[] pvector;
   delete fftr2c;

   return;

}

//============================================================================================

void PulseTools::ComplexToRealIFFT(const vector<TComplex>& inComp, vector<double>& outRe)
{
    int n = inComp.size(); 
    if(n == 0)
    {
      cerr <<"PulseTools::ComplexToRealIFFT - ERROR! empty pulse passed into this function" << endl;
      exit(1);
    }

    double *re_pvector = new double[inComp.size()];
    double *im_pvector = new double[inComp.size()];

    double re,im;
    int i;
  
    //copying std vector back into arrays to pass to IFFT
    for (i=0; i < n; i++){
       re_pvector[i] = inComp[i].Re();  
       im_pvector[i] = inComp[i].Im();  
    }
   
    TVirtualFFT *ifftc2r = TVirtualFFT::FFT(1,&n,"C2R ES"); //this should be the opposite of FFT
    ifftc2r->SetPointsComplex(re_pvector, im_pvector);
    ifftc2r->Transform();
    ifftc2r->GetPointComplex(0,re,im);
    outRe.push_back(re/sqrt((double)n)); //DC component
   
    for(i=1; i<n ;i++)
    {
       ifftc2r->GetPointComplex(i,re, im);
       outRe.push_back(re/sqrt((double)n));
    }

   delete[] re_pvector;
   delete[] im_pvector;
   delete ifftc2r;

   return;
}

// ==========================================================================================

//This function returns the PSD in units of pulseNorm/sqrt(Hz), so note that this means an 
//extra squareroot.  This function also assumes that the original vector (pulseVector) is real, 
//and so exploits the symmetry between fourier components of positive and negative frequencies
void PulseTools::Time2PSD(const vector<double>& pulseVector, const double& fs, vector<double>& outPSD)
{
   int nBins = pulseVector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::Time2PSD - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   // check if nBins even or odd
   bool isEven = false;
   if (nBins%2==0) isEven=true;
   
   // Do FFT
   vector<TComplex> outCFFT;
   RealToComplexFFT(pulseVector,outCFFT);
   uint i;
   
   // size of PSD depends if even or odd trace length original trace
   int nBinsPSD = 0;
   if (isEven) nBinsPSD = nBins/2+1;
   else nBinsPSD = (nBins+1)/2;
   
 
   // We  want the 2-sided distribution in pulseNorm/sqrt(Hz), exploiting symmtery for +/- frequency components
   // being carefull that 
   //    1. DC compoment should not be doubled for +/- frequency
   //    2. Max frequency (Nyquist) should not be doubled for +/- frequency (case trace length even)


   // First let's calculate the squared norm
   
   // DC component...
   outPSD.push_back(outCFFT[0].Re()*outCFFT[0].Re()); // DC component
   
   // .. and the rest
   for (i=1;i<nBinsPSD;i++)
     outPSD.push_back(outCFFT[i].Re()*outCFFT[i].Re() 
		      + outCFFT[nBins-i].Im()*outCFFT[nBins-i].Im());
   
   
   // Then normalize properly in pulseNorm/sqrt(Hz)
   for(i=0; i<outPSD.size(); i++)
     {
       // DC AND  Nyquist component for even length case-> no factor 2
       if (i==0 || ((i==outPSD.size()-1) && isEven)) {
	 outPSD[i] = sqrt(outPSD[i]/fs);
       } else 
	 outPSD[i] = sqrt(2.0*outPSD[i]/fs);
     }
   
   return;
}

// ==========================================================================================

//NOTE: This function expects the PSD to be in units of V/sqrt(Hz)
// -- with phase information
void PulseTools::PSD2FFT(const vector<double>& psdvector,const vector<double>& anglevector, vector<TComplex>& outComp,
			 int traceLengthType)
{
   int nBins = psdvector.size();

   if(nBins == 0)
   {
      cerr <<"PulseTools::PSD2FFT - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   
   // Is original trace even or odd?
   bool isEven = false;
   if (traceLengthType!=0){
     if (traceLengthType==2) isEven= true;
   } else {
     // Orinal trace unknow so will guess... 
     // Works only for certain trace length such as power of  2 lengtht
     if ((nBins+1) % 2 == 0) isEven = true; 
   }
   
   
   uint i;
   vector<TComplex> outTemp;
   TComplex atemp,btemp;
   
   
   // Build a temporary vector 
   // Normalize with sqrt(2.0) but this will have to be taken out for DC and 
   // Nyquist component (case even trace length)
   //push out the first half into a temporary vector
   for(i=0;i<nBins;i++){
     btemp(0,anglevector[i]);
     atemp = btemp.Exp(btemp)*psdvector[i]/sqrt(2.0);
     outTemp.push_back(atemp);
   }
   


   // Now Build output vector
   if (isEven)
     { //even number of elements in original pulse
       
       // 1. DC component  (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[0]);
       
       // 2. Rest of First half without Nyquist
       for(i=1;i<nBins-1;i++)
	 outComp.push_back(outTemp[i]);

       // 3. Nyquist (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[nBins-1]); 

       // 4. second half complex conjugate
       for(i=nBins-2;i>0;i--){
	 outComp.push_back(outTemp[i].Conjugate(outTemp[i]));
       }
     } 
   else 
     {  //odd number of elements in original pulse
       
        // 1. DC component  (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[0]); //DC component
       
       //now copy first half into output vector 
       for(i=1;i<nBins;i++){
	 outComp.push_back(outTemp[i]);
       }
       // second half
       for(i=nBins-1;i>0;i--){
	 outComp.push_back(outTemp[i].Conjugate(outTemp[i]));
       }
     }
   
   return;
   
}


//NOTE: This function expects the PSD to be in units of V/sqrt(Hz)
// -- without phase information
void PulseTools::PSD2FFT(const vector<double>& psdvector, vector<double>& outComp,
			 int traceLengthType)
{
   int nBins = psdvector.size();
  
   if(nBins == 0)
   {
      cerr <<"PulseTools::PSD2FFT - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   // Is original trace even or odd?
   bool isEven = false;
   if (traceLengthType!=0){
     if (traceLengthType==2) isEven= true;
   } else {
     // Orinal trace unknow so will guess... 
     // Works only for certain trace length such as power of  2 lengtht
     if ((nBins+1) % 2 == 0) isEven = true; 
   }
   
   
   int i;
   vector<double> outTemp;
   TComplex atemp;

       
   // Build a temporary vector  (similar than PSD2FFT with phase information)
   // Normalize with sqrt(2.0) but this will have to be taken out for DC and 
   // Nyquist component (case even trace length)
   for(i=0;i<nBins;i++){
     atemp = psdvector[i]/sqrt(2.0); // complex number not really required but using same code...
     outTemp.push_back(atemp);
   }
   
   // Now Build output vector
   if (isEven)
     { //even number of elements in original pulse
       
       // 1. DC component  (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[0]);

       // 2. Rest of First half without Nyquist
       for(i=1;i<nBins-1;i++)
	 outComp.push_back(outTemp[i]);

       // 3. Nyquist (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[nBins-1]); 

       // 4. second half 
       for(i=nBins-2;i>0;i--){
	 outComp.push_back(outTemp[i]);
       }

     } 
   else 
     {  //odd number of elements in original pulse

        // 1. DC component  (removing sqrt(2.0))
       outComp.push_back(sqrt(2.0)*outTemp[0]); //DC component
       
       //now copy first half into output vector 
       for(i=1;i<nBins;i++){
	 outComp.push_back(outTemp[i]);
       }
       // second half
       for(i=nBins-1;i>0;i--){
	 outComp.push_back(outTemp[i]);
       }
     }
   
   return;
   
}



//This function returns the correlation of two pulses in units of V^2/Hz (or Amp^2/Hz for phonon pulses).
//There is no additional squareroot taken, unlike the PSD!
void PulseTools::Time2FFTCov(const vector<double>& gPulseVector, const vector<double>& hPulseVector, 
			     const double& fs, vector<double>& outFFTCorr_Re, vector<double>& outFFTCorr_Im)
{
   int nBins = gPulseVector.size();

   if(nBins == 0)
   {
      cerr <<"PulseTools::Time2FFTCorr - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   if((uint)nBins != hPulseVector.size() )
   {
      cerr <<"PulseTools::Time2FFTCorr - ERROR! pulses do not have same number of bins" << endl;
      exit(1);
   }

   vector<TComplex> gOutCFFT;
   RealToComplexFFT(gPulseVector,gOutCFFT);

   vector<TComplex> hOutCFFT;
   RealToComplexFFT(hPulseVector,hOutCFFT);

   uint i;

   // --- DC component ---

   TComplex corrProd(gOutCFFT[0]*hOutCFFT[0].Conjugate(hOutCFFT[0]));
   outFFTCorr_Re.push_back(corrProd.Re()/fs); 
   outFFTCorr_Im.push_back(corrProd.Im()/fs); 


   // --- push out the rest ---

   for(i=1;i<(int) ((double)nBins+1.0)/2.0;i++)
   {
      TComplex tempProd = gOutCFFT[i]*(hOutCFFT[i].Conjugate(hOutCFFT[i]));
      outFFTCorr_Re.push_back(tempProd.Re()/fs);
      outFFTCorr_Im.push_back(tempProd.Im()/fs);
   }

   return;
}

// ==========================================================================================

vector<double> PulseTools::Differentiate(const vector<double> &pulsevector)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::Differentiate - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   vector<double> differentialTrace;

   for (int binCtr = 0; binCtr< (int) pulsevector.size(); binCtr++)
   { 
      if(binCtr > 0)
	 differentialTrace.push_back(pulsevector[binCtr] - pulsevector[binCtr-1]);
      else
	 differentialTrace.push_back(pulsevector[binCtr]); //0th bin is not a derivative value
   }

   return differentialTrace;
}      

// ==========================================================================================
//Determines delay for saturated pulses with respect to Template start time, tempStart
//Takes raw pulse as input, with baseline and std as separate inputs.
//Checks for saturation by comparing to the saturation value, satValue
double PulseTools::SaturationDelay(const vector<double>& pulsevector,double bs, double std, int tempStart,double satValue)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::SaturationDelay - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   double delay = 0.; //default delay is 0
   int nadc;
   int satBin = 0; //bin at which saturation occurs
   int satBinSTD = 0;
   double qstd = std;
   
   //Find bin at which un-subtracted pulse begins to saturate
   for(nadc=0;nadc<nBins;nadc++){
      if(pulsevector[nadc] >= satValue){
	 satBin = nadc;
	 break;
      }
   }
   vector<double> bsvector;
   for(nadc=0;nadc<nBins;nadc++){
     bsvector.push_back(pulsevector[nadc]-bs);
   }
   if (IsSaturated(pulsevector,satValue)){ //check if pulse actually saturates
      for(nadc=0;nadc<nBins;nadc++){
	if (bsvector[nadc]<5.0*qstd) satBinSTD = nadc; //start of pulse is defined when pulse crosses 5*std above baseline
         else break;
      }
      if (satBinSTD >= satBin) delay = (double)-1.0*tempStart;//negative of template start
      else delay = (double)satBinSTD - tempStart; //difference between pulse start and template start
   }

   return delay;
}

//=========================================================================================
//Returns number of bins at or above saturation point
int PulseTools::NumBinsSaturation(const vector<double>& pulsevector, double satValue)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::NumBinsSaturation - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   int satCounter,nadc;
   int pulsesize = pulsevector.size();

   satCounter = 0; //initialize counter

   for(nadc=0;nadc<pulsesize;nadc++){
      if(pulsevector[nadc]>=satValue) satCounter++;
   }

   return satCounter;
}

//Returns number of bins below minimum value
int PulseTools::NumBinsMinimum(const vector<double>& pulsevector, double minValue)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::NumBinsMinimum - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   int minCounter,nadc;
   int pulsesize = pulsevector.size();

   minCounter = 0; //initialize counter

   for(nadc=0;nadc<pulsesize;nadc++){
      if(pulsevector[nadc]<minValue) minCounter++;
   }

   return minCounter;
}

//=========================================================================================
//Get first saturation bin
double PulseTools::FirstSaturationBin(const vector<double>& pulsevector, double satValue)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::FirstSaturationBin - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   int nadc,firstbin;
   int pulsesize = pulsevector.size();
   firstbin = pulsesize; //initialize counter
   for(nadc=0;nadc<pulsesize;nadc++){
      if(pulsevector[nadc]>=satValue) {firstbin = nadc; break; }
   }

   return (double)firstbin;
}

//=========================================================================================
//Determine if pulse is negative
bool PulseTools::IsNegative(const vector<double>& pulsevector)
{
   int nBins = pulsevector.size();
   if(nBins == 0)
   {
      cerr <<"PulseTools::IsNegative - ERROR! empty pulse passed into this function" << endl;
      exit(1);
   }

   bool isnegative = 0;
   double maxadc = MaxADC(pulsevector);
   vector<double> pulsevector_flip;
   int pulsesize = pulsevector.size();
   for(int i = 0;i<pulsesize;i++){
      pulsevector_flip.push_back(-1.0*pulsevector[i]);
   }
   if (MaxADC(pulsevector_flip)>maxadc) isnegative = 1;
  
   return isnegative;
}

//=========================================================================================

//makes a histogram out of aVector with limits 0 to xscale*nBins
TH1D PulseTools::Vector2TH1D(const vector<double>& aVector, const string& aHistoName,
			     const double xscale)
{
   //Note we do not check for zero length here because sometime these vectors are zero on purpose
   
   int nBins = aVector.size();

   //now initialize a histogram, x axis is bins
   string histName = Form("%s", aHistoName.c_str());
   TH1D tempHistogram(histName.c_str(), histName.c_str(), nBins, 0, ((double)nBins)*xscale);
   
   //now fill the histogram with the values in each bin of the vector
   //remembering that bins start at 1 in ROOT
   for(int binCtr=1; binCtr<=nBins; binCtr++)
   {
      tempHistogram.SetBinContent(binCtr, aVector[binCtr-1]);
   }
   
   return tempHistogram;   
}

//Makes a histogram out of a map with both key (x) and value (y) as double.
//This routine could be made smarter to figure out xmin and xmax, but right
//now this must be passed in (and provides more flexibility)
TH1D PulseTools::Map2TH1D(const map<double,double>& aMap, const string& aHistoName,
			  const double& xmin, const double& xmax)
{
   int nBins = aMap.size();

   //now initialize a histogram, x axis is bins
   string histName = Form("%s", aHistoName.c_str());
   TH1D tempHistogram(histName.c_str(), histName.c_str(), nBins, xmin, xmax);
   
//    cout <<"xmin = " << xmin <<", xmax = " << xmax << endl;

   //now fill the histogram with the values in each bin of the vector
   //remembering that bins start at 1 in ROOT
   map<double, double>::const_iterator mapItr = aMap.begin();

   for( ; mapItr != aMap.end(); mapItr++)
   {
     cout<<"x = " << mapItr->first <<", y = " << mapItr->second << endl; 
     tempHistogram.Fill(mapItr->first, mapItr->second);
   }
   
   return tempHistogram;   
}

//=========================================================================================
  
vector<double> PulseTools::TH1D2Vector(const TH1D& aHisto) 
{

  // number of bins (histrogram x axis)
  int nBins = aHisto.GetNbinsX(); 
 
  // create and fill vector (remembering that histograms bins start at 1 in ROOT)

  vector<double> tempVector; 
  for(int binCtr=1; binCtr<=nBins; binCtr++)
   {
      tempVector.push_back(aHisto.GetBinContent(binCtr));
   }
   
  return tempVector;

}

TGraph* PulseTools::Vector2TGraph(const vector<double>& aVector, 
				  double xscale, double xstart)
{
  vector<double> x(aVector.size(), xstart);
  transform(x.begin(),x.end()-1,x.begin()+1,bind1st(plus<double>(),xscale));
  TGraph* graph = new TGraph(x.size(), &x[0], &aVector[0]);
  graph->SetEditable(false);
  return graph;
}


TGraphErrors* PulseTools::Vector2TGraphErrors(const vector<TComplex>& aVector, double xyscale)
{
  vector<double> x(aVector.size(), 0.);
  transform(x.begin(),x.end()-1,x.begin()+1,bind1st(plus<double>(),xyscale));
  vector<double> yre(aVector.size(), 0.);
  vector<double> yim(aVector.size(), 0.);
  for(int i=0;i<(int)aVector.size();i++){ yre[i]=aVector[i].Re(); yim[i]=aVector[i].Im(); }
  TGraphErrors* graph = new TGraphErrors(x.size(), &x[0], &x[0], &yre[0], &yim[0]);
  graph->SetEditable(false);
  return graph;
}


TGraphErrors* PulseTools::Vector2TGraphErrors(const vector<double>& aVector, double xyscale)
{
  vector<double> x(aVector.size(), 0.);
  transform(x.begin(),x.end()-1,x.begin()+1,bind1st(plus<double>(),xyscale));
  vector<double> yre(aVector.size(), 0.);
  vector<double> yim(aVector.size(), 0.);
  for(int i=0;i<(int)aVector.size();i++){ yre[i]=aVector[i]; }
  TGraphErrors* graph = new TGraphErrors(x.size(), &x[0], &x[0], &yre[0], &yim[0]);
  graph->SetEditable(false);
  return graph;
}

vector<TComplex> PulseTools::TGraphErrors2Vector(const TGraphErrors* aGraph)
{
  vector<TComplex> outVec(aGraph->GetN(),0.0);
  double x,y;
  TComplex z;
  int maxel=0;
  for(int i=0;i<aGraph->GetN();i++){
    aGraph->GetPoint(i,x,y);
    if((int)x == (int)y){
      if((int)x>=(int)outVec.size())
        outVec.resize((int)x+1);
      if((int)x>maxel)
        maxel=(int)x;
      outVec[(int)x]= TComplex(aGraph->GetErrorX(i),aGraph->GetErrorY(i));
    }
  }
  return outVec;
}

//=========================================================================================

//The following few functions allow for a toy simulation to study the accuracy of delay and amplitude 
//returned by the fitter using noise pulses.  Select for random trigger events to use with "ConstructFakePulse".  
//It uses the noise pulses and pulse template to build a "fake pulse" from the templates and noise pulses.  
//It is recommended to use the version with position dependence when studying performance of phonon fitters.
//Note that these routines expect the norm to be in the same units as whatever normalization the noise pulse 
//is in when its passed to these routines.

vector<double> PulseTools::ConstructFakePulse(double norm, double delay, const vector<double>& aNoisePulse,
						       const vector<double>& aPulseTemplate)
{

   //The fake pulse to be returned by this routine
   vector<double> aFakePulse;

   //This will store the pulse histograms
   //TFile f_out("dumppulses.root", "update");

   //Initializations
   int traceLength = aNoisePulse.size();

   //cout <<"delay = " << delay << endl;

   // ===== Construct the pulse! ========
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < traceLength; binItr++)
   {
      //for a positive delay
      if(delay >= 0)
      {
	 if(binItr < delay)
	 {
	    aFakePulse.push_back(aNoisePulse[binItr]);
	 }
	 if(binItr >= delay)
	 {
	    int shiftBin = binItr - (int)delay;
	    aFakePulse.push_back(norm*aPulseTemplate[shiftBin] + aNoisePulse[binItr] );
	 }
	 
      }//endif positive delay

      //for a negative delay
      else
      {
	 int shiftBin = binItr - (int)delay;
	 
	 if(shiftBin < traceLength)
	 {
	    aFakePulse.push_back(norm*aPulseTemplate[shiftBin] + aNoisePulse[binItr] );
	 }
	 else
	 {
	    aFakePulse.push_back(aNoisePulse[binItr]);
	 }
	 
      } //end if negative delay
      
   } //end loop over bins
   
   //Store the fake pulse
   //(PulseTools::Vector2TH1D(aFakePulse,"FakePulse")).Write();
   //f_out.Close();
   
   return aFakePulse;
}

vector<double> PulseTools::ConstructPositionDependentFakePulse(double norm, double delay, const vector<double>& aNoisePulse,
							       const vector<double>& aPulseTemplate, int seed)
{
  //add position dependence to the pulse template
  vector<double> positionDependentTemplate = AddPositionDependence(aPulseTemplate, seed);

  //create fake pulse
  return (ConstructFakePulse(norm, delay, aNoisePulse, positionDependentTemplate));

}

//add simple toy model of position dependence to the pulse, with random amplitude
//model is a linear rise to fractionalAmp of pulse template over interval1 bins, 
//a linear fall to fractionalAmp*pdAmp2 of puse template over the next interval2 bins 
//and a return to baseline over interval3 bins.  
//Parameters are determined empiraclly (somewhat AD HOC model), seems to match ok though
//This routine assumes the pulse template is normalized to 1
//Note that the random number generator gives the same sequence of values for the same seed
vector<double> PulseTools::AddPositionDependence(const vector<double>& aPulseTemplate, int seed)
{

  vector<double> aPositionDependentPulse;
  aPositionDependentPulse = aPulseTemplate;

  //Choose a random amplitude for the position dependence
  TRandom3 rand(seed);
  double relativeAmp = rand.Uniform(-1, 1);
  double fractionalAmp = relativeAmp*0.15; 

  //define the position dependence template
  int templateOffset = 504; //the bin where position dependence begins
  int interval1 = 5; 
  int interval2 = 10;
  int interval3 = 75;
  int intervaltot = interval1 + interval2 + interval3;
  double pdAmp1 = 1.0; //amplitude of PD, rising side of residual
  double pdAmp2 = 0.25; //amplitude of undershoot

  for(int binItr = 0; binItr < aPositionDependentPulse.size(); binItr++)
  {
    //do nothing in prepulse region
    if(binItr < templateOffset || binItr >= (templateOffset + intervaltot)) 
    {
      continue;
      //aPositionDependentPulse[binItr] = 0; //debugging only
    }

    //linear rise to amplitude over interval1 bins
    if(binItr <(templateOffset+interval1) && binItr >=templateOffset) 
    {
      aPositionDependentPulse[binItr] += (fractionalAmp/interval1)*(binItr-templateOffset); 
    }

    //linear fall to -pdAmp2 over next interval2 bins
    if(binItr <(templateOffset+interval1+interval2) && binItr >=(templateOffset+interval1)) 
    {
      aPositionDependentPulse[binItr] += fractionalAmp - (pdAmp1+pdAmp2)*(fractionalAmp/interval2)*(binItr-(templateOffset+interval1)); 
    }

    //linear rise to baseline over next interval3 bins
    if(binItr <(templateOffset+intervaltot) && binItr >=(templateOffset+interval1+interval2)) 
    {
      aPositionDependentPulse[binItr] += (fractionalAmp*pdAmp2/interval3)*(binItr-(templateOffset+interval1+interval2)) 
	                                 - pdAmp2*fractionalAmp; 
    }
  
  }

  return aPositionDependentPulse;
}


//=========================================================================================

//  trick to be able to have the template function inside the source instead of the header...
//  Here's a reference http://stackoverflow.com/questions/972152/how-to-create-a-template-function-within-a-class-c [ANV]
//  Basic idea -- if a SPECIFIC implementation is not instantiated or called within the .cxx code you compile into the object
//  (.o) file, this object file will lack the symbol defining this function and therefore whatever links against it will have
//  and undefined reference.  
//  Honestly, because of the above facts, I'm not sure it makes sense to think of these as "template functions" they are more like
//  overloaded functions in which you used a trick to save some typing.  -- if you don't have a member below with the explicit signature
//  that you are going to use, you can't use the function [ANV].
//  It's even more annoying because if you actually use a specific template signture in this .cxx file it will be included in the (.o) file
//  but it will be unstable in the sense that if you remove that call in the class implementation, then try to use that signature externally
//  you will have an undefined reference, AND you will go crazy looking for it because you didn't change the code which the compiler reports
//  as having the undefined reference. [ANV]

template TComplex PulseTools::pulseDot(const vector<TComplex> &veca,const vector<TComplex> &vecb);
template vector<TComplex> PulseTools::pulseNSMult(const vector<TComplex> &veca,const vector<TComplex> &vecb);
template vector<TComplex> PulseTools::pulseScale(const vector<TComplex> &vec,TComplex scale);
template double PulseTools::pulseDot(const vector<double> &veca,const vector<double> &vecb);
template vector<double> PulseTools::pulseNSMult(const vector<double> &veca,const vector<double> &vecb);
template vector<double> PulseTools::pulseScale(const vector<double> &vec,double scale);

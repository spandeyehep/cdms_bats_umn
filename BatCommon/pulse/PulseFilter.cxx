//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: PulseFilter
//Authors: L. Hsu, M. Kos
//Description:  A class containing functions that return a filtered pulse
//given a cutoff frequency and sample rate. The Butterworth lowpass, hipass,  
//and bandpass filters are implemented    
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#include "iir.h" //from http://www.exstrom.com/journal/sigproc

#include "PulseTools.h"
#include "PulseFilter.h"

using namespace std;

vector<double> PulseFilter::ButterLowPass(const vector<double> &tracevector, double sampleRate, 
					  double freqCut, int filterOrder)
{
  int n = filterOrder;                       // filter order
  //int sff = 1;                               // scale flag: 1 to scale, 0 to not scale ccof - not currently used
  int i;                                     // loop variables
  double fcf = 2.0*freqCut/sampleRate;       // cutoff frequency (fraction of pi)
  double sf;                                 // scaling factor
  double *dcof;                              // d coefficients
  int *ccof;                                 // c coefficients
  //int vsize = (int) tracevector.size();    // not currently used
  vector<double> filtervector;
  vector<double> tempVector;

  if(fcf >= 1 || fcf <= 0)
  {
    cout <<"PulseFilter::ButterLowPass ERROR!  2*cutoff/sampleRate must be inside the interval [0,1], instead the value is " << fcf 
	 << endl;
    exit(1);
  }

  //1. calculate the d coefficients (these are "a" coeffs in Matlab)
  dcof = dcof_bwlp( n, fcf );
  if( dcof == NULL )
  {
     cout <<"PulseFilter::ButterLowPass ERROR! Unable to calculate d coefficients" << endl;
     exit(1);
  }


  //2. calculate the c coefficients (these are "b" coeffs in Matlab)
  ccof = ccof_bwlp( n );
  if( ccof == NULL )
  {
     cout <<"PulseFilter::ButterLowPass ERROR! Unable to calculate c coefficients" << endl;
     exit(1);
  }


  sf = sf_bwlp( n, fcf ); /* calculate scaling factor for the c coefficients */
  
  /* debug only - Output the file header */
//   printf("# Butterworth lowpass filter coefficients.\n" );
//   printf("# Produced by bwlp.\n" );
//   printf("# Filter order: %d\n", n );
//   printf("# Cutoff freq.: %1.15lf\n", fcf );
//   printf("# Scaling factor: %1.15lf\n", sf );

  /* debug only - Output the c, d coefficients */
//   printf("%d\n", n+1 ); /* number of c coefficients */
//   if( sff == 0 )
//      for( i = 0; i <= n; ++i)
// 	printf("%d\n", ccof[i] );
//     else
//        for( i = 0; i <= n; ++i)
// 	  printf("%1.15lf\n", (double)ccof[i]*sf );
  
//   printf("%d\n", n+1 );  /* number of d coefficients */
//   for( i = 0; i <= n; ++i )
//      printf("%1.12lf\n", dcof[i] );


  //not entirely sure why this is needed - LLH
  double *a = new double[filterOrder+1];
  double *b = new double[filterOrder+1];

  for(i=0;i<=n;i++){
     a[i] = dcof[i];
     b[i] = sf*ccof[i];
  }

  //3. Using FiltFilt instead, to attempt to deal with startup transients - LLH
  vector<double> filtfiltPulse = FiltFilt(n, a, b, tracevector, sampleRate, freqCut);

  //4. cleanup
  free( dcof );
  free( ccof );
  
  delete [] a;
  delete [] b; 


  return filtfiltPulse;
}

vector<double> PulseFilter::ButterHighPass(const vector<double> &tracevector, double sampleRate, 
					   double freqCut, int filterOrder)
{
  int n = filterOrder;                       // filter order
  //int sff = 1;                             // scale flag: 1 to scale, 0 to not scale ccof - not currently used
  int i;                                     // loop variables
  double fcf = 2.0*freqCut/sampleRate;       // cutoff frequency (fraction of pi)
  double sf;                                 // scaling factor
  double *dcof;                              // d coefficients
  int *ccof;                                 // c coefficients
  //int vsize = (int) tracevector.size();    // not currently used
  vector<double> filtervector;
  vector<double> tempVector;
  
  if(fcf >= 1 || fcf <= 0)
  {
    cout <<"PulseFilter::ButterHighPass ERROR!  2*cutoff/sampleRate must be inside the interval [0,1], instead the value is " << fcf 
	 << endl;
    exit(1);
  }
  
  //1. calculate the d coefficients a coeffs (these are "a" coeffs in Matlab)
  dcof = dcof_bwhp( n, fcf );
  if( dcof == NULL )
    {
      perror( "Unable to calculate d coefficients" );
      exit(1);
    }

  //2. calculate the c coefficients (these are "b" coeffs in Matlab)
  ccof = ccof_bwhp( n );
  if( ccof == NULL )
    {
      perror( "Unable to calculate c coefficients" );
      exit(1);
    }

  sf = sf_bwhp( n, fcf ); /* scaling factor for the c coefficients */

  double *a = new double[filterOrder+1];
  double *b = new double[filterOrder+1];

  for(i=0;i<=n;i++){
    a[i] = dcof[i];
    b[i] = sf*ccof[i];
  }
  
  //3. Using FiltFilt instead, to deal with startup transients - LLH
  vector<double> filtfiltPulse = FiltFilt(n, a, b, tracevector, sampleRate, freqCut);
  
  //4. cleanup
  free( dcof );
  free( ccof );
  
  delete [] a;
  delete [] b; 
  
  return filtfiltPulse;
}

vector<double> PulseFilter::ButterBandPass(const vector<double> &tracevector, double sampleRate, 
					   double lowfreqCut, double highfreqCut, int filterOrder)
{
  int n = filterOrder;                       // filter order
  //int sff = 1;                               // scale flag: 1 to scale, 0 to not scale ccof - not currently used
  int i;                                     // loop variables
  double fcf1 = 2.0*lowfreqCut/sampleRate;   // low cutoff frequency (fraction of pi)
  double fcf2 = 2.0*highfreqCut/sampleRate;  // high cutoff frequency (fraction of pi)
  double sf;                                 // scaling factor
  double *dcof;                              // d coefficients
  int *ccof;                                 // c coefficients
  //int vsize = (int) tracevector.size();    // not currently used
  vector<double> filtervector;
  vector<double> tempVector;

  if(fcf1 >= 1 || fcf1 <= 0 || fcf2 >= 1 || fcf2 <= 0 )
  {
    cout <<"PulseFilter::ButterBandPass ERROR!  2*cutoff/sampleRate must be inside the interval [0,1]"
	 << endl;
    exit(1);
  }
  
  //1. calculate the d coefficients (these are "a" coeffs in Matlab)
  dcof = dcof_bwbp( n, fcf1, fcf2 );
  if( dcof == NULL )
    {
      perror( "Unable to calculate d coefficients" );
      exit(1);
    }

  //2. calculate the c coefficients (these are "b" coeffs in Matlab)
  ccof = ccof_bwbp( n );
  if( ccof == NULL )
    {
      perror( "Unable to calculate c coefficients" );
      exit(1);
    }

  sf = sf_bwbp( n, fcf1, fcf2 ); /* scaling factor for the c coefficients */

  double *a = new double[2*filterOrder+1]; //twice as many coefficients compared to hipass and lowpass
  double *b = new double[2*filterOrder+1];

  for(i=0;i<=2*n;i++){
    a[i] = dcof[i];
    b[i] = sf*ccof[i];
  }
  
  //3. Using FiltFilt instead, to deal with startup transients - LLH
  vector<double> filtfiltPulse = FiltFilt(2*n, a, b, tracevector, sampleRate, lowfreqCut);
  
  //4. cleanup
  free( dcof );
  free( ccof );
  
  delete [] a;
  delete [] b; 
  
  return filtfiltPulse;
}

vector<double> PulseFilter::ButterBandStop(const vector<double> &tracevector, double sampleRate, 
					   double lowfreqCut, double highfreqCut, int filterOrder)
{
  int n = filterOrder;                       // filter order
  //int sff = 1;                               // scale flag: 1 to scale, 0 to not scale ccof - not currently used
  int i;                                     // loop variables
  double fcf1 = 2.0*lowfreqCut/sampleRate;   // low cutoff frequency (fraction of pi)
  double fcf2 = 2.0*highfreqCut/sampleRate;  // high cutoff frequency (fraction of pi)
  double sf;                                 // scaling factor
  double *dcof;                              // d coefficients
  double *ccof;                              // c coefficients
  //int vsize = (int) tracevector.size();    // not currently used
  vector<double> filtervector;
  vector<double> tempVector;

  if(fcf1 >= 1 || fcf1 <= 0 || fcf2 >= 1 || fcf2 <= 0 )
  {
    cout <<"PulseFilter::ButterBandStop ERROR!  2*cutoff/sampleRate must be inside the interval [0,1]"
	 << endl;
    exit(1);
  }
  
  //1. calculate the d coefficients (these are "a" coeffs in Matlab)
  dcof = dcof_bwbs( n, fcf1, fcf2 );
  if( dcof == NULL )
    {
      perror( "Unable to calculate d coefficients" );
      exit(1);
    }

  //2. calculate the c coefficients (these are "b" coeffs in Matlab)
  ccof = ccof_bwbs( n, fcf1, fcf2 );
  if( ccof == NULL )
    {
      perror( "Unable to calculate c coefficients" );
      exit(1);
    }

  sf = sf_bwbs( n, fcf1, fcf2 ); /* scaling factor for the c coefficients */

  double *a = new double[2*filterOrder+1];
  double *b = new double[2*filterOrder+1];

  for(i=0;i<=2*n;i++){
    a[i] = dcof[i];
    b[i] = sf*ccof[i];
  }
  
  //3. Using FiltFilt instead, to deal with startup transients - LLH
  vector<double> filtfiltPulse = FiltFilt(2*n, a, b, tracevector, sampleRate, lowfreqCut);
  
  //4. cleanup
  free( dcof );
  free( ccof );
  
  delete [] a;
  delete [] b; 
  
  return filtfiltPulse;
}

//This routine designed to mimic Matlab filtfilt function. Filter forward and backwards to remove phase shift.  
//Attempt to handle boundaries by prepending several filter lengths of the flipped, inverted pulse, 
//shifted so that baselines match.  Then pass this off to PulseFilter::Filt()
vector<double> PulseFilter::FiltFilt(int order, double *a, double *b, 
				     const vector<double> &tracevector, double sampleRate, double freqCut)
{
   vector<double> filterVector;
   vector<double> tempVector;        //to hold intermediate filtered pulse
   vector<double> extendVector;      //to hold intermediate filtered pulse
   vector<double> finalVector;       //final filtered pulse returned by function
   vector<double> extendVectorBegin; //to handle boundary of pulse
   vector<double> extendVectorEnd;   //to handle boundary of pulse

   uint nBinsExpand = int(3.0*sampleRate/freqCut);
   uint vsize = (int) tracevector.size();

   //nBinsExpand could be longer than the original pulse!, limit (somewhat arbitrarily size - 1) for speed
   if(nBinsExpand >= vsize) 
     { 
       cout <<"PulseFilter::FiltFilt WARNING!  sampleRate/freqCut exceeds limit where boundary conditions for filtering can be kept manageable.  Filtered pulse may suffer from artifacts of filtering algorithm"
	    << endl;
       nBinsExpand = tracevector.size()-1; 
     }

   // prepend nBinsExpand that are inverted, reflected and baseline shifted to match DC offset of true pulse
   extendVectorBegin = CreateExtension(tracevector, nBinsExpand, "pre");
   extendVector = extendVectorBegin;
   for(uint i=0; i<vsize; i++){
      extendVector.push_back(tracevector[i]);
   }

   // postpend nBinsExpand that are inverted, reflected and baseline shifted to match DC offset of true pulse
   extendVectorEnd = CreateExtension(tracevector, nBinsExpand, "post");
   for(uint i=0; i<extendVectorEnd.size(); i++){
      extendVector.push_back(extendVectorEnd[i]);
   }

   // forward filter the extended pulse
   filterVector = Filt(order, a, b, extendVector);
 
   // reverse the filtered, extended pulse
   for(uint i=0; i< filterVector.size(); i++){
      tempVector.push_back(filterVector[filterVector.size()-1-i]);
   }

   // then filter again to remove phase shift
   filterVector = Filt(order, a, b, tempVector); 

   // reverse again to get back to original orientation, only copy the original bins
   for(uint i=nBinsExpand; i < filterVector.size()-(nBinsExpand); i++){
      finalVector.push_back(filterVector[filterVector.size()-1-i]);
   }

   return finalVector;
}

//This function is a utility for correct filtering at boundary of pulse.  It creates a fake extension
//to the pulse by inverting, reflecting and baseline shifting to match DC offset of true pulse
vector<double> PulseFilter::CreateExtension(const vector<double> &pulseVector, int nBinsExpand,
                                            const string& whichEnd)
{
   vector<double> extensionVector;
//   vector<double> tempVector; //hold intermediate calculation

   if(whichEnd != "pre" && whichEnd != "post")
   {
      cout <<"PulseFilter::CreateExtension ERROR!  Unrecognized string passed as whichEnd."
	   << endl;
      exit(1);
   }

   //Copy nBinsExpand# of bins into extensionVector (don't duplicate the first bin for the reflection)
   if(whichEnd == "pre")
   {
      //Note this is also being reflected as it is copied
      for(int binCtr=0; binCtr < (nBinsExpand); binCtr++)
      {
	 if((uint)binCtr >= pulseVector.size()-1) { break; } //get out of loop if end of pulse is reached
	 
	 extensionVector.push_back(pulseVector[nBinsExpand - binCtr]);
      }      
   }
   else
   {
      //Note this is also being reflected as it is copied
      for(int binCtr=1; binCtr <= nBinsExpand; binCtr++)
      {
	 if((uint)binCtr >= pulseVector.size()-1) { break; } //get out of loop if start of pulse is reached

	 extensionVector.push_back(pulseVector[pulseVector.size() -1 - binCtr]);
      }
   }


   // invert the extension
   extensionVector = PulseTools::InvertPulse(extensionVector);

   // match the DC offset
   // shift extension so that it matches the endpoint of the original pulse
   double baselineShift = (whichEnd == "pre" ? 2.0*pulseVector[0] : 2.0*pulseVector[pulseVector.size()-1]);
  
   for(uint binCtr=0; binCtr < extensionVector.size(); binCtr++)
   {
      extensionVector[binCtr] = extensionVector[binCtr] + baselineShift;
   }

   return extensionVector;
}

//This routine originally a copy of Matlab's filter function
//This code is originally from:  http://mechatronics.ece.usu.edu/yqchen/filter.c
vector<double> PulseFilter::Filt(int order, double *a, double *b, const vector<double> &filtvector)
{
  int i,j;
  vector<double> outvector;
  for(i=0;i<(int)filtvector.size();i++){
    outvector.push_back(0.0);
  }
  outvector[0]=b[0]*filtvector[0];
  for (i=1;i<order+1;i++)
    {
      outvector[i]=0.0;
      for (j=0;j<i+1;j++)
	outvector[j]=outvector[i]+b[j]*filtvector[i-j];
      for (j=0;j<i;j++)
	outvector[i] = outvector[i]-a[j+1]*outvector[i-j-1];
    }
  /* end of initial part */
  for (i=order+1;i<(int)filtvector.size();i++)
    {
      outvector[i]=0.0;
      for (j=0;j<order+1;j++)
	outvector[i]=outvector[i]+b[j]*filtvector[i-j];
      for (j=0;j<order;j++)
	outvector[i]=outvector[i]-a[j+1]*outvector[i-j-1];
    }
  return outvector;
} /* end of filter */





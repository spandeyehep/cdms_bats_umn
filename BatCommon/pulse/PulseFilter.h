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

#ifndef PULSEFILTER_H
#define PULSEFILTER_H

#include <iostream>
#include <vector>

#include "TObject.h"
#include "TMath.h"
#include "math.h"

using namespace std;

typedef unsigned int uint;

//! A class containing pulse filtering funtions.  So far just has Butterworth filter
class PulseFilter
{

public:
      
      static vector<double> ButterLowPass(const vector<double> &tracevector,double sampleRate, 
					  double freqCut,  int filterOrder = 2);
      
      static vector<double> ButterHighPass(const vector<double> &tracevector,double sampleRate, 
					   double freqCut,  int filterOrder = 2);  

      static vector<double> ButterBandPass(const vector<double> &tracevector,double sampleRate, 
					   double lowfreqCut, double highfreqcut, int filterOrder = 2);  

      static vector<double> ButterBandStop(const vector<double> &tracevector,double sampleRate, 
					   double lowfreqCut,  double highfreqcut, int filterOrder = 2);  

 private:

      static vector<double> Filt(int order, double *a, double *b, const vector<double> &filtvector); //single filter
      static vector<double> FiltFilt(int order, double *a, double *b, const vector<double> &filtvector, 
				     double sampleRate, double freqCut); //forward and backwards filter, w/ boundary handling
      static vector<double> CreateExtension(const vector<double> &pulseVector, int nBinsExpand, 
					    const string& whichEnd);

};
#endif /* PULSEFILTER_H */


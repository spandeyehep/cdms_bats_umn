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
//  Ben Loer, Jan 10, 2012: Change confusing all-static class to namespace
//  A.N. Villano, May 15, 2012:  Add conversion from TGraphErrors
//                       and add template functions to do dot-products,
//                       vector multiplcations with no sum, and pulse
//                       scaling
//  N. Mast, Nov 14, 2017:  Add sloped baseline subtraction function: SlopedBaselineSub
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef PULSETOOLS_H
#define PULSETOOLS_H

#include <iostream>
#include <vector>
#include <map>
#include "math.h"

#include "TObject.h"
#include "TMath.h"
#include "TVirtualFFT.h"
#include "TComplex.h"
#include "TH1D.h"
#include "TGraph.h"
#include "TGraphErrors.h"


using namespace std;
typedef unsigned int uint;
typedef struct minmax_struct {
  double max;
  double max_bin;
  double min;
  double min_bin;
  double minmax;
} minmax_struct;

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

//! A namespace containing general pulse operations 
namespace PulseTools
{

  // simple pulse calculations coded with templates
  
  template<class Type> Type pulseDot(const vector<Type> &veca,const vector<Type> &vecb);

  template<class Type> vector<Type> pulseNSMult(const vector<Type> &veca,const vector<Type> &vecb);

  template<class Type> vector<Type> pulseScale(const vector<Type> &vec,Type scale);
  
  
  // simple pulse calculations

  vector<double> Normalize(const vector<double> &pulsevector, const double& norm);

  vector<double> Scale(const vector<double> &pulsevector, const double& scale);

  vector<double> InvertPulse(const vector<double> &pulsevector);

  vector<double> SumPulses(const vector<double> &pulsevector1, const vector<double> &pulsevector2);

  double         Area(const vector<double> &pulsevector, 
		      int lowbin = -1, int hibin = -1);

  double         Baseline(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1);

  vector<double> BaselineSub(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1);
  
  vector<double> SlopedBaselineSub(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1, int endBins = -1);
  double 	 SlopedBaseline(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1, int endBins = -1);
            
  double         MaxADC(const vector<double> &pulsevector,
			int lowbin = -1, int hibin = -1);
  double         MaxADCPoint(const vector<double> &pulsevector,
			     int lowbin = -1, int hibin = -1);

  float          MinMax(const vector<double> &pulsevector,
			int lowbin = -1, int hibin = -1);
  
  
  minmax_struct  MinMaxStruct(const vector<double> &pulsevector,
			      int lowbin = -1, int hibin = -1);
  
  double         PileUp(const vector<double> &pulsevector, int ndiv);

  vector<double> Differentiate(const vector<double> &pulsevector);      

  bool IsNegative(const vector<double>& pulsevector);
      
  // RMS/Standard Dev. 

  double RMS(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1);

  double Std(const vector<double> &pulsevector, int lowbin = -1, int hibin = -1); //same as above, except for normalization
      
  // Walk times

  double RTFTWalkTimeP(const vector<double> &pulsevector,double percent_time, 
		       int lowbin = -1, int hibin = -1);

  double RTFTWalkTimeQ(const vector<double> &pulsevector,double percent_time, double stdev, 
		       int lowbin = -1, int hibin = -1);

  double PipeFitterWalk(const vector<double> &pulsevector,double testvalue, 
			int startbin, int endbin); //What Pipefitter uses in the time domain fit, possibly remove later
          
  // Histogramming

  TH1D Vector2TH1D(const vector<double>& aVector, const string& aHistoName,
		   const double xscale = 1.0); 

  TH1D Map2TH1D(const map<double, double>& aMap, const string& aHistoName, 
		const double& xmin, const double& xmax); 

  vector<double> TH1D2Vector(const TH1D& aHisto); 
      
  //TGraph conversion
  TGraph* Vector2TGraph(const vector<double>& aVector, 
			double xscale=1., double xstart=0.);
      
  inline vector<double> TGraph2Vector(const TGraph* aGraph)
  { return vector<double>(aGraph->GetY(),aGraph->GetY()+aGraph->GetN()); }

  //TGraphError conversion  (probably only useful for our storage of diagonal matricies) [ANV]
  //currently we use TGraphErrors as a Root class which can represent sparse Complex matricies
  TGraphErrors* Vector2TGraphErrors(const vector<TComplex>& aVector, double xyscale=1.0);
  TGraphErrors* Vector2TGraphErrors(const vector<double>& aVector, double xyscale=1.0);

  //grab the "diagonal" of our matrix stored in TGraphError [ANV]
  vector<TComplex> TGraphErrors2Vector(const TGraphErrors* aGraph);


  // FFT's used for optimal filter - fftw, encapsulated in ROOT version

  void RealToComplexFFT(const vector<double>& pulsevector, vector<TComplex>& outComp);

  void ComplexToRealIFFT(const vector<TComplex>& inComp, vector<double>& outRe);

  void Time2PSD(const vector<double>& pulseVector, const double& fs, vector<double>& outPSD); 

  void PSD2FFT(const vector<double>& psdvector, const vector<double>& anglevector, vector<TComplex>& outComp,
	       int traceLengthType = 0); //with angles
  void PSD2FFT(const vector<double>& psdvector, vector<double>& outComp, 
	       int traceLengthType = 0); //without angles

  //correlation terms in fourier space between pulses g and h
  void Time2FFTCov(const vector<double>& gPulseVector, const vector<double>& hPulseVector, 
		   const double& fs, vector<double>& outFFTCorr_Re, vector<double>& outFFTCorr_Im); 

  // Saturation Calculations

  bool   IsSaturated(const vector<double> &pulsevector,double satvalue);

  double SaturationDelay(const vector<double>& pulsevector,double bs, double std, int tempStart = 512 ,double satValue = 4091.);

  int NumBinsSaturation(const vector<double>& pulsevector, double satValue);  //Number of saturation bins

  int NumBinsMinimum(const vector<double>& pulsevector, double minValue);  //Number of bins below set ADC value

  double FirstSaturationBin(const vector<double>& pulsevector, double satValue = 4091.);  
  
  
  vector<double> FourBasic(const vector<double> &pulsevector, int nn, int isign);
  vector<double> RealFFT(const vector<double> &pulsevector, int n, int isign = 1,int itype = 0);


  // Pulse simulations - construct a fake pulse from template and noise trace, with known delay and amplitude
  // Note that these routines expect the norm to be in the same units as whatever normalization the noise pulse 
  // is in when its passed to these routines.

  vector<double> ConstructFakePulse(double norm, double delay, const vector<double>& aNoisePulse,
				    const vector<double>& aPulseTemplate);                  // for study of charge fitters

  vector<double> ConstructPositionDependentFakePulse(double norm, double delay, 
						     const vector<double>& aNoisePulse,
						     const vector<double>& aPulseTemplate,
						     int seed); // recommended for study of phonon fitters
  
  vector<double> AddPositionDependence(const vector<double>& aPulseTemplate, int seed); //mostly just supporting routine

};

#endif /* PULSETOOLS_H */

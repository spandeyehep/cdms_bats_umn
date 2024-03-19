////////////////////////////////////////////////////////////////////////////////// 
//Class:  OptimalFilterPhononNS Class
//Author: A.N. Villano 
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on single pulse using the non-stationary covariance matrix to subtract "noise"
//  associated with position dependence.
// 
//
//File Import By: A.N. Villano 
//Creation Date:  May 13, 2012
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#ifndef OptimalFilterPhononNS_H
#define OptimalFilterPhononNS_H

#include <iostream>
#include <vector>
#include <map>

//ROOT classes
#include "TComplex.h"
#include "TH1D.h"

//CDMSBATS math library (inherits from blas sparse matrix of complex nums)
#include "SprseMatrix.h"

//CDMSBATS standard tools
#include "TCDMSAnalysis.h"
#include "PulseTools.h"

//boost libraries FIXME should package in BatMath tools
#include <boost/numeric/bindings/traits/ublas_vector.hpp>
//#include <boost/numeric/bindings/traits/ublas_sparse.hpp>
#include <boost/numeric/bindings/umfpack/umfpack.hpp>

//namespace ublas = boost::numeric::ublas;
namespace umf = boost::numeric::bindings::umfpack;

typedef ublas::vector<complex<double> > blasVec;

using namespace std;

//!This is the OptimalFilterPhononNS Class.  It serves as a template for all other analysis classes
class OptimalFilterPhononNS : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      OptimalFilterPhononNS();  
      ~OptimalFilterPhononNS(); //destructor 

      //Set parameters
      //void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here
      int GetVerbosity() const {return fVerbose; }
      double GeteV() const     { return fAmp; }
      double GetAmp() const     { return fAmp; }
      double GetChisq() const  { return fChisq; }
      double GetDelay() const  { return fDelay; }
      double GetAmpBig() const     { return fAmpBig; }
      double GetChisqBig() const  { return fChisqBig; }
      double GetDelayBig() const  { return fDelayBig; }
      bool GetIsRandom() const { return fIsRandom; }
      bool GetPrintRandom() const { return fPrintRandom; }
      bool GetCalcSTDOF() const { return fCalcSTDOF; }
      bool GetDoMaxAmp() const { return fDoMaxAmp; }  //set by initializing window properly


      //Set parameters
      void SetVerbosity(int v) { fVerbose = v; return; }
      void SetVerbosityRange(int a, int b); 
      void SetSampleTime(double dt) { fdT = dt; return; }
      void SetWindows(int window1, int window2) 
      {  fwindow1 = window1; 
	 fwindow2 = window2; 
	 return; }
      void SetChiWidth(int chihalfwidth) {  fchihalfwidth = chihalfwidth; return; }
      void SetIsRandom(bool IsRandom) { fIsRandom = IsRandom; return; }
      void SetPrintRandom(bool PrintRandom) { fPrintRandom = PrintRandom;
          if(!fPrintRandom && fIsRandom){
           fVerbose=0;
          }
          return; }


      //for loading in through extData manager
      void LoadTemplates(const vector<TComplex>& pulseTemplateFFT);
      void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const SprseMatrix& COVpd, const SprseMatrix& COVbase);
      void LoadThresholds(const double& filterThresh, const int& nsofcutoff);
      void LoadOFParams(const double& OFAmpsP, const double& OFDelayP);



   private:

      //dangerous causes calculation of standard OF only; call before DoNSOF
      void SetCalcSTDOF(bool CalcSTDOF) { fCalcSTDOF = CalcSTDOF; return; }

      void ConstructRQList();

      //define private functions and data members here
      
      //fit values
      double fAmp;
      double fAmp0;
      double fChisq;
      double fDelay;
      double fAmpBig;
      double fChisqBig;
      double fDelayBig;

      //parameters
      double fdT;
      int    fwindow1; //in adc bins
      int    fwindow2; //in adc bins
      double fOFAmpsP; //standard phonon optimal filter value
      double fOFDelayP; //standard phonon optimal delay value
      double ffilterThresh;  //if OF amplitude is low enough, dont do NSOF
      int fchihalfwidth; //chisquare window width in adc bins
      int fnscutoff;   //dont consider correlations at all freq
      int fSizeCOVpt;
      int fSizeCOVbase;
      int fNBinsTemplates;
      int fVerbose;
      int fVerboseN1;
      int fVerboseN2;
      bool fDoDnCastCalc;  //can do matricies in two parts if dncast (not yet implemented)
      bool fDoInverseCalc;  //can do by explicit inverse (not yet implemented) 
      bool fDoNSFilter;
      bool fTemplatesLoaded;
      bool fNormalizationsLoaded;
      bool fOFParamsLoaded;
      bool fThreshParamsLoaded;
      bool fIsRandom;
      bool fPrintRandom;
      bool fCalcSTDOF;  //dangerous, only set by private method
      bool fDoMaxAmp; 
      vector<TComplex> fOptimalFilter;
      vector<double> fChisquare;

      //templates and norms
      double fNormFFT;
      double fSigToNoiseSq;
      SprseMatrix fCOVpd;
      SprseMatrix fCOVbase;
      vector<TComplex> fPulseTemplateFFT;

      //do actual calcuation in various situations (4 options total)
      bool DoNSOF(const vector<double>& aPulse, bool &DoInverseCalc,bool &DoDnCastCalc); 


      //inefficient and will remove by absorbing things into CDMSBATS Vector object
      //but for now need to convert between types.
      blasVec convVecTComplexToSTL(vector<TComplex>&);
      vector<TComplex> convSTLToVecTComplex(blasVec&);
      vector<TComplex> conj(vector<TComplex>&);
      blasVec conj(blasVec&);
      blasVec pulseNSMultBlas(const blasVec&,const blasVec&); 

      //functions for computation of chisquare
      //and various minimizations
      void findMax(vector<double> &list,double &max,double &time,TH1D* ref);
      void findMin(vector<double> &list,double &min,int &itime);
      vector<double> getChiSquare(SprseMatrix &covTotalSparse,const blasVec &pulsefft,const blasVec &templatefft,vector<int> &tdel_win,vector<double> &amps,double &norm);



      

};

#endif /* OptimalFilterPhononNS_H */
